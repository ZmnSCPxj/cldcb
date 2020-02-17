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
