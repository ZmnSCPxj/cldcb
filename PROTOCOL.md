
* `give_recognition_code` 0
  * tlv: `recognition_code` 0, length 64
  * Client to server.
  * Associates the client ID with the given recognition code.
  * Server rejects this message if not recognized client ID.

* `request_recognition_codes` 2
  * Client to server.
  * Requests a list of client IDs and recognition codes.
  * Server always gives this message.

* `response_recognition_codes` 4
  * tlv: `cid_recognition_codes` 0, length n * 97
  * Server to client.
  * Returns the client IDs and recognition codes.
  * 0 or more are sent in response to
    `request_recognition_codes`.
* `response_recognition_codes_end` 6
  * Server to client.
  * Tells the client there are no more recognition codes.

--

* `ping` 8
  * tlv: `pointless_data` 0, length n
  * Either direction.
  * Pointless message to keep the TCP connection alive.
  * The receiver should reply with a `pong`.
* `pong` 9
  * tlv: `pointless_data` 0, length n
  * Either direction.
  * Pointless message to keep the TCP connection alive.
  * The receiver should ignore this message.

--

* `request_incremental` 32
  * tlv: `data_version` 0, length 4
  * Client to server.
  * Tells the server that a new incremental update
    is available.
  * The client will immediately send `incremental_chunk`
    messages soon.
  * Server rejects this message (disconnects) if not
    recognized client ID.
* `response_reupload` 36
  * Server to client.
  * Sent in response to the `request_incremental` if
    the server wants to get a reupload.
  * If the client receives this message, it should
    send `reupload_chunk` messages after sending the
    `incremental_chunk` messages.
* `acknowledge_upload` 38
  * Server to client.
  * Sent in response to `incremental_end` if the server
    did not send `response_reupload`.
    Sent in response to `reupload_end` if the server
    *did* send `response_reupload`.
  * Tells the client that the server has backed up all
    the data that has been sent.

* `incremental_chunk` 68
  * tlv: `chunk` 0, length n
  * Client to server.
  * Provides a chunk of incremental data to the server.
  * Server rejects this message if it did not receive a
    `request_incremental`.
* `incremental_end` 70
  * Client to server.
  * Tells the server that there is no more incremental
    data to send.

* `reupload_chunk` 52
  * tlv: `chunk` 0, length n
  * Client to server.
  * Provides a chunk of reupload data to the server.
  * Server rejects this message if it is did not send
    `response_reupload`.
* `reupload_end` 54
  * Client to server.
  * Tells the server that there is no more reupload data.

The process goes this way:

* Client sends `request_incremental`.
* The server checks the backup if it exists and the
  `data_version` is good.
  If it does not exist, it sends `response_reupload`.
* Client sends `incremental_chunk` terminated by
  `incremental_end` (without waiting for a server
  response).
* If the client received a `response_reupload`, the
  client sends `reupload_chunk` terminated by
  `reupload_end`.
* The server writes all the data to disk, then syncs,
  then sends `acknowledge_upload`.

Thus the client logic goes this way:

* Send `request_incremental`.
* Send `incremental_chunk` terminated by
  `incremental_end`.
* Receive a message.
  * If it is `response_reupload`, it sends
    `reupload_chunk` terminated by
    `reupload_end`, then receives a message
    that should be `acknowledge upload`, then
    it know the backup succeeded.
  * If it is `acknowledge_upload` then it
    knows the backup succeeded.

The server logic goes this way:

* Receive a message, and if it is
  `request_incremental`, continue this logic.
* Check if the backup is existent and
  `data_version` is ok.
  * In the case both are okay:
    * Receive `incremental_chunk`s and write to
      disk.
    * On receiving `incremental_end`, sync and
      then respond with `acknowledge_upload`.
  * In the case one or the other are not true:
    * Send `response_reupload`.
    * Receive `incremental_chunk`s and cache
      them.
    * On receiving `incremental_end`, receive
      `reupload_chunk`s and write them to disk.
    * On receiving `reupload_end`, get the
      incremental updates from the cache and
      write them to disk.
    * Then sync and send `acknowledge_upload`.

--

* `request_backup_data` 112
  * tlv: `cid` 0, length 33
    * The client backup data that the client wants to
      fetch.
  * Client to server.
  * Asks the server to return the backup data.
  * The server will start by sending
    `response_backedup_reupload_chunk` and so on.
* `response_backedup_reupload_chunk` 114
  * tlv: `chunk` 0, length n
  * Server to client.
  * Gives the client a chunk of the reupload data.
* `response_backedup_reupload_end` 116
  * Server to client.
  * Tells the client the reupload data has finished.
* `response_backedup_incremental_new` 118
  * tlv: `data_version` 0, length 4
  * Server to client.
  * Tells the client that a new increment is starting.
    Succeeding `response_backedup_increment_chunk`
    messages will belong to the given `data_version`.
* `response_backedup_incremental_chunk` 120
  * tlv:: `chunk` 0, length n
  * Server to client.
  * Gives the client a chunk of the increment data.
* `response_backedup_incremental_endall` 122
  * Server to client.
  * Tells the client that all of the incremental
    data has been sent.

