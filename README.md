C-Lightning Dynamic Channel Backup Mechanism
============================================

> WARNING: This `README` is "what I want cldcb to be", not what it is right now!

This backup mechanism is composed of many parts:

* `cldcb-server` which can safely be run on somebody else computer,
  and which will store the channel backups.
* `cldcb-plugin` which is a C-Lightning plugin that handles
  `db_write` hooks.
* `cldcb-client` which is used by a C-Lightning operator to set up
  the plugin and to recover from remote backup.

Currently C-Lightning supports PostgreSQL and SQLite3 for its
channel state database.
`cldcb` only supports SQLite3.
If you can handle PostgreSQL, you should probably set up replication
with PostgreSQL rather than this mechanism.

Setup
=====

First you need to install `cldcb-server` on a remote server.
You only need to trust the remote server to the extent of it keeping
your data around indefinitely, so you can run this on a VPS.
It should be accessible over the open IP network, because accessing
over Tor is slow and will slow down your C-Lightning operations, but
you could also run a backup server over an .onion service if you are
fine with the performance degradation.

You want the `cldcb-server` to be run at each startup of your remote
server.
This means installing it in your `init` system somehow.

The `cldcb-server` will create a server identifier, this is a 66-hex-digit
string whose first digit will be `5`.

Then, you need to set up the `cldcb-server` to allow 1 or more nodes
to back up their data there, as well as where in the filesystem to
store the data.
You do this by providing a "client identifier" to the server.

To create a client identifier, install `cldcb-client` (which also
installs `cldcb-plugin`) on the computer(s) with your Lightning
node(s).
Then run:

    cldcb-client newclient $CLIENT_PRIVKEY_FILE

(`$CLIENT_PRIVKEY_FILE` is any filename you want.)

`cldcb-client` will save the identifier private key in the given
file, then print out the client identifier.
You can also open the `$CLIENT_PRIVKEY_FILE` (it is a text file)
to see the client identifier, it is the hex string starting with
`id =` and with the hex string always starting with the hex digit
`c`.
The `$CLIENT_PRIVKEY_FILE` will look like:

    # cid = c3df949fa268776a789f321051211a9183fb9c2891b8dc2742cfc9107ec7f9d23f
    # cpk = e49802ca07d9f3a579afefca067dd28f22c572de8756d8ef372c53873f82b25e

The first is the client identifier you need to give to the backup
server.
The second is a private key and should not be given to anyone, not
even the backup server.

Then, on the backup server, run:

    cldcb-server add $CLIENT_IDENTIFIER

This tells the server about a new client that will save backups on
the server.

Afterwards, you are ready to install the plugin into your C-Lightning.
While your C-Lightning node is running, run:

    cldcb-client installplugin --lightning-dir=$LIGHTNINGDIR \
                               --client-privkey=$CLIENT_PRIVKEY_FILE

Follow the prompts and tell `cldcb-client` how to access the
`cldcb-server`.

This will create a new plugin named `cldcb-plugin-wrapper` in your
C-Lightning installation, and also trigger C-Lightning to reload
plugins so that the backup plugin will be started and you can
start uploading your backups to the `cldcb-server`.

After this you can safely delete the `$CLIENT_PRIVKEY_FILE`;
its content will be saved in `cldcb-plugin-wrapper`.

Finally, create a **secure offline** backup of your `hsm_secret`
file.
This can be found in your `$LIGHTNINGDIR`.
In principle you could hex-dump it (`xxd hsm_secret`) and then
write the hex down on a piece of paper, it is just 64 hex characters.

***Without a copy of `hsm_secret` your CLDCB data cannot be
recovered (or stolen, for that matter).***

Single-Step Setup
-----------------

(TODO)

If you have `sshd` installed on the backup server, and `ssh` installed
locally (which is likely), then `cldcb-client` can do a single-step
setup.

    cldcb-client install --lightning-dir=$LIGHTNINGDIR

This will issue the `cldcb-server add` command automatically to the
server you provide, via `ssh`.
This will cause `ssh` to ask for your username and password for the
remote server.

This will create a client identifier and install it in your server.

Recovery
========

***Do NOT run `lightningd` normally yet.***

First, recover your `hsm_secret`.
Create a `$LIGHTNINGDIR` and place your recovered `hsm_secret`
file there.

Then run:

    cldcb-client recover --lightning-dir=$LIGHTNINGDIR

Follow the prompts and tell `cldcb-client` how to access the
`cldcb-server`.
This will recreate the database and also re-add a plugin
named `cldcb-plugin-wrapper`.

With both the `hsm_secret` and the database recovered, you can
now resume running the Lightning node.

Special HSMs
------------

In the future there may be HSMs that can actually work with
C-Lightning.
In that case, you might not have an actual `hsm_secret` file
to recover.
You will have to go check with your exact HSM about how to
safely back up its signing key.

To support such HSMs, you can use the `--alt-hsmd=` flag:

    # Need to run inside the $LIGHTNINGDIR
    cd $LIGHTNINGDIR
    cldcb-client recover --alt-hsmd=$ALTHSMD

Where `$ALTHSMD` is what you pass in to `lightningd` via the
`--subdaemon=hsmd:` option.

(At the time of this writing, no such HSM exists, but there
is always that possibility in the future.)

How It Works
============

A problem is that `db_write` hook is really an awful hook;
you can do almost nothing within it.
You cannot get options from C-Lightning before `db_write`
could trigger, you cannot query anything from C-Lightning
while `db_write` is in-flight, and so on.

Further, it would be preferable if the server does not need to
know the identifier for the Lightning network node it is keeping
backups for.

Thus, we have *three* keypairs:

* The key of the node, i.e. the node identifier.
* The key of the `cldcb-plugin`, i.e. the client identifier.
* The key of the `cldcb-server`, i.e. the server identifier.

When a `cldcb-plugin` is installed into a C-Lightning node, we
generate a keypair for it to serve as the client identifier.
The server will only allow client identifiers it has been set
up to receive from.
The data saved by the plugin is encrypted to be decryptable only
by the node private key itself.

Authentication with the server simply requires that the plugin
prove to the backup server that it has control of the client
private key corresponding to the client identifier that was
configured at the server.

Thus, the plugin never needs to query anything from the
C-Lightning node:
all it needs is the client private key (which is separate from
the node keypair).

Now an issue is that we cannot do anything in the plugin, not
even C-Lightning command-line options, since options are provided
during `init`, but `db_write` hooks can get invoked before that.
So we cannot even know *where* the `cldcb-server` *is* by
looking at the C-Lightning options, because `db_write` can occur
before the options are passed to the plugin!

Thus, `cldcb-client` creates a shell script `cldcb-plugin-wrapper`
that is the actual plugin that is executed by C-Lightning.
This contains the `cldcb-plugin` client *private* key, the node
public key, and how to contact the `cldcb-server`.
The wrapper just `exec`s into `cldcb-plugin`.

`cldcb-plugin` is passed the `$0` argument of `cldcb-plugin-wrapper`,
which is the location of `cldcb-plugin-wrapper`.
The **actual** data (client privkey, server contact details...)
is then parsed by `cldcb-plugin` from the shell script comments.
This is because modern operating systems store the entire command line
used to invoke a process, so we cannot safely pass in private keys
and etc etc via the command line.
If you are running on a multi-user computer you do *not* want people
to `ps` and see a private key.
(But if you are doing this and you are not the sole administrator, do
note `root` can easily read any file, including `hsm_secret`.)
We cannot pass it in via `stdin` either, because `stdin` is where
C-Lightning communicates with the plugin!

Thus, when `cldcb-plugin` is starting up, it will start communicating
with the `cldcb-server` and authenticate itself.
Afterwards, as it handles `db_write`s, it just pushes the data to the
server.

The data pushed to the server is encrypted using an asymmetric
encryption scheme.
This is just ECDH on a random ephemeral scalar with the node public
key, followed by standard HKDF and symmetric stream crypto and MAC
and so on.
The plugin encrypts the queries (which can now only be decrypted by
the node that knows the `hsm_secret`) and sends the encrypted queries
to the server.

Part of the `db_write` hook interface is a `data_version` field.
This is sent openly to the server.
Every `data_version` should be exactly one higher than the previous
`data_version`; if not, or if the server has no database from the
client, the server will ask the client for a copy of the database.
The `cldcb-plugin` will then use another ephemeral scalar to
asymmetrically encrypt the database and send it to the server.
Periodically, if the server notes that the encrypted queries on
top of the database are larger than the database, it will
request the full database anyway regardless.

Recovery is now difficult.
Presumably, recovery happens because the entire C-Lightning node went
permanently down, and the only thing that was saved was the
`hsm_secret`.
Thus, even the client identifier (even the client public key!) is
presumably lost as well.

To let a newly-recovered C-Lightning node identify which client
identifier it was using before backup, the C-Lightning node must
provide a signature that commits to the client public key.
When the plugin connects to the server, it sends this signature
as well, and the server keeps a list of client public keys and
the signature.

However, we should note that with a signature and the message it
signs, it is possible to derive the public key that originally
signed the message (i.e. public key recovery).
To avoid leaking the C-Lightning node public key, we instead
perform the following ritual:

* We perform an ECDH of the client key and the C-Lightning node
  key.
* The C-Lightning node signs the resulting shared secret.

The above commits to the client public key, as a different client
keypair would yield a different shared secret; it also prevents the
server from guessing the message that is signed, preventing it
from being able to grind the node public key.

(It would have been better for the client private key to be a
hardened-derived key from the node private key, however the
interface of `db_write` is awful and we cannot query anything
from `lightningd`.
With this technique we only need a signature once, which we can
do during installation of the plugin to the C-Lightning plugin
directory, not require continuous access to the `hsm_secret`
(which might not exist as a file if a "true" HSM is designed
someday).)

On recovery:

* The recovered C-Lightning node requests the list of client
  public keys and associated signatures.
* For each entry in the list, the C-Lightning node tries to
  validate the associated signature with its own public key.
  If it validates, then the C-Lightning node knows which client
  public key was being used for backup.
* We can even recover the client private key by encrypting it
  in an asymmetric encryption with the node public key, so that
  it can be read only by knowledge of the node private key.
  * Thus, the node private key is all that is needed to recover
    the entire C-Lightning setup (including `cldcb-plugin`) from a
    `cldcb-server` which keeps serving the backup data.
* The server never learns which C-Lightning node was using any
  particular client identifier.

`cldcb-client` will either read the `hsm_secret`, or communicate with
a `lightning_hsmd`-alike program to get signatures (for communicating
with the `cldbc-server`) and ECDH operations (for decrypting the
data).
