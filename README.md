C-Lightning Dynamic Channel Backup Mechanism
============================================

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

Then, you need to set up the `cldcb-server` to allow 1 or more nodes
to back up their data there, as well as where in the filesystem to
store the data.
This means providing the Lightning node ID(s) of the node(s) you want
to keep backups for.

Then, you will need to install `cldcb-client` (which also installs
`cldcb-plugin`) on the computer(s) with your Lightning node(s).
While your C-Lightning node is running, run:

    cldcb-client install --lightning-dir=$LIGHTNINGDIR

Follow the prompts and tell `cldcb-client` how to access the
`cldcb-server`.

This will create a new plugin named `cldcb-plugin-wrapper` in your
C-Lightning installation, and also trigger C-Lightning to reload
plugins so that the backup plugin will be started and you can
start uploading your backups to the `cldcb-server`.

Finally, create a **secure offline** backup of your `hsm_secret`
file.
This can be found in your `$LIGHTNINGDIR`.
In principle you could hex-dump it and then write the hex down
on a piece of paper, it is just 64 hex characters.

***Without a copy of `hsm_secret` your CLDCB data cannot be
recovered (or stolen, for that matter).***

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

During installation of the plugin, we generate a public-private
keypair, then ask the Lightning node to sign the generated public
key using `signmessage`.
We use that public key to tell `cldcb-server` that we are an
authorized representative of the Lightning node.

Thus, authentication with the server goes this way:

* We present the Lightning node public key.
  The server checks that it is in the list of nodes it is keeping
  backups for.
* We present `cldcb-plugin` public key.
* We present the signature of the Lightning node that signs
  the `cldcb-plugin` public key.
  This effectively tells the server that this is a delegate of
  the Lightning node.
  * What is actually signed is a string of the public key in
    hex, with a prefixed scare-message of "DO NOT SIGN THIS MESSAGE
    UNLESS YOU WANT TO LOSE ALL YOUR FUNDS ".

Remember, we cannot query the C-Lightning node *anything* while a
`db_write` is in-flight, not even a `signmessage`, so we cannot sign
every message to `cldcb-server` that way.
Worse, we cannot even know *where* the `cldcb-server` *is* by
looking at the C-Lightning options, because `db_write` can occur
before the options are passed to the plugin!

Thus, `cldcb-client` creates a shell script `cldcb-plugin-wrapper`
that is the actual plugin that is executed by C-Lightning.
This contains the `cldcb-plugin` *private* key, the signature from
C-Lightning, the node public key, and how to contact the
`cldcb-server`.
The wrapper just `exec`s into `cldcb-plugin`.

`cldcb-plugin` is passed the `$0` argument of `cldcb-plugin-wrapper`,
which is the location of `cldcb-plugin-wrapper`.
The **actual** data (privkey, signature, server contact details...)
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

On recovery, the `cldcb-client` will request the encrypted database
and the queries.
It will either read the `hsm_secret`, or communicate with an
`lightning_hsmd`-alike program to get signatures (for communicating
with the `cldbc-server`) and ECDH operations (for decrypting the
data).
