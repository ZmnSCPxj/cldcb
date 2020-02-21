`cldcb-client` and `cldcb-plugin`
=================================

`cldcb-client`
--------------

The client-side `cldcb-client` program helps C-Lightning node
operators set up the `cldcb-plugin` program as a plugin.
It asks the C-Lightning node operator about server details, then
creates a wrapper shell script for the `cldcb-plugin` program,
installing the wrapper into the plugins-dir of a C-Lightning node.

The plugin has to know some details about the node, in particular
it needs to know the node ID, as well as the server ID.

`cldcb-plugin`
--------------

The plugin binary is given the wrapper shell script as first
command-line argument.
The shell script contains the details of the plugin in the form
of comments, which `cldcb-plugin` parses.

The plugin launches a number of threads:

* `stdin` handler - Handles incoming requests from C-Lightning.
  * This thread in particular encodes and encrypts the incoming
    `db_write` hooks and prepares it for sending to the server(s).
  * It also encrypts the database if any server requests for the
    database.
* `stdout` serializer - Ensures that entire JSON objects are
  sent to C-Lightning; in particular the logger also sends to this.
* Server handler - Connects to the server and establishes a session
  with them, then sends any pending data to it.
  * The session adds a second layer of encryption; the server can
    decrypt this layer, but the node-specific data is still encrypted
    so that only the node privkey can decrypt it.

The plugin binary shell script has a format much like the following:

    #! /bin/sh

    ## Lines starting with ## and #! are true comments.
    ## Lines starting with a single # are key-value pairs.
    ## Other lines are ignored, but probably have some meaning
    ## to the shell interpreter.

    ## Client identifier. Start with c2 or c3.
    # cid = c3df949fa268776a789f321051211a9183fb9c2891b8dc2742cfc9107ec7f9d23f
    ## Client private key; the private key behind the client identifier.
    # cpk = e49802ca07d9f3a579afefca067dd28f22c572de8756d8ef372c53873f82b25e

    ## Node identifier. Start with 02 or 03.
    ## This is the pubkey of the Lightning node.
    # nid = 02df949fa268776a789f321051211a9183fb9c2891b8dc2742cfc9107ec7f9d23f
    ## Node signature; this is the signature of the Lightning node.
    # nsig = df949fa268776a789f321051211a9183fb9c2891b8dc2742cfc9107ec7f9d23fdf949fa268776a789f321051211a9183fb9c2891b8dc2742cfc9107ec7f9d23fff

    ## Server identifier. Start with 52 or 53.
    # sid = 52df949fa268776a789f321051211a9183fb9c2891b8dc2742cfc9107ec7f9d23f
    ## Server host. IPv4, IPv6, or .onion, plus optional port.
    # shost = 192.168.1.1:29735
    ## SOCKS5 proxy, used to communicate with the server.
    ## Optional; if unspecified, not used.
    # socks5 = 127.0.0.1:9050

    ## The line below actually starts the plugin binary.
    exec cldcb-plugin -- "$0"

