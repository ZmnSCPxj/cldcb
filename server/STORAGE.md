Storage Format
==============

Each backup has a single file.
The initial part of a stored backup is a sequence of chunks,
called the "reupload" chunks.
This is the "raw" data, encrypted by the client.

This is then followed by a set of "incremental" updates.
This is the delta, starting from the raw data, of each
update of the data.

Finally, the incremental updates are followed by a footer.
This contains information about the most recent incremental
update.

Reupload Section
----------------

This is fairly straightforward:
This is only a sequence of chunks.

A chunk is just 2 bytes of the chunk length (big-endian),
followed by the chunk data.

The sequence of reupload chunks is terminated by a chunk of
0 length.
This implies of course that no reupload chunk can validly
have a length of 0.

Incremental Section
-------------------

The incremental section is composed of one or more
incremental updates.

Each incremental update has its own footer.

An incremental update is composed of:

* 4 bytes, a 32-bit big-endian `data_version`.
* A sequence of chunks, each chunk of which is:
  * 2 bytes of the 16-bit big-endian chunk length.
* A terminating chunk, i.e. a chunk of size 0, i.e.
  2 bytes that are `0x00 0x00`.
* A footer.

The footer contains data about the most recent
incremental section.

* 8 bytes, a 64-bit big-endian `last_length`, the
  size of the incremental update (without the footer
  itself).
  This is the sum of the 4-byte `data_version`, the
  chunk lengths and chunk data, and the terminating
  chunk.
* 4 bytes, a 32-bit big-endian copy of the
  `data_version`.
* 2 bytes, the number of incremental updates minus 1.
* 32 bytes, the SHA-2 256-bit hash of the above 14
  bytes of footer.

The hash serves as a checksum to ensure that the
footer data was correctly written.
This is because the server may crash before it can
completely write an update to the backup storage.
In that case, the footer data might not exist.

Writing To The Backup
=====================

The backup is typically written to on an incremental
update.
This involves the client sending a new `data_version`
followed by chunks of the incremental update.

First, the server reads the footer and validates that
the footer is valid (by going to the end of file minus
46 bytes, hashing 14 bytes, then checking if the next
32 bytes is the hash of the data).
If the footer is invalid, we just ask the client for a
new upload.
(Or we could re-read the file and find the most recent
incremental update that is a complete one, and truncate
to just after that incremental update and then process
as below, that could be done at some point in the
future.)

Otherwise, we check the footer data.

* If the number of incremental updates is already at
  the maximum number of updates configured, we ask the
  client for a reupload anyway.
* If the `data_version` of the incoming update is
  equal to the last `data_version` in the footer, we
  truncate the file, removing the most recent incremental
  update available (which is why the length of the
  incremental update is stored in the footer as well).
  Then we append the incremental update with its own
  footer.
  (NOTE: as an edge case, if this is the only incremental
  update so far (i.e. its count is 0) we ask for a reupload
  instead, as we cannot safely delete the first incremental
  update, since every archive must have at least one.)
* If the `data_version` of the incoming update is
  equal to one plus the last `data_version` (including
  wraparound), we just append the incremental update
  with its own footer.
* Otherwise we ask the client for a reupload.

In case of an incremental update without a reupload,
we record the file length prior to writing anything
to the incremental update.
If the client gets disconnected while writing the
incremental update we truncate to this previous
length, which ensures the end of the file still has
a valid footer.

In case of a reupload, we store the incremental update
in another file temporarily, then write the reupload
data in another tempoarary, then append the incremental
update.
Then finally we rename over the archive file and delete
the incremental update file.
