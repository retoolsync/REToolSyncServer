# Protocol

This directory contains documentation and example JSON payloads for the REToolSync protocol.

## Architecture

The protocol has been designed to make it as simple as possible to implement REToolSync clients. `REToolSyncServer` is a central server (optionally hosted on the public internet). Its only job is to broadcast any message sent to the `/REToolSync` endpoint back to all clients, except the client sending the initial message.

The functionality of `REToolSync` is implemented by _intelligence providers_, usually plugins for reverse engineering tools like IDA, Ghidra, Binary Ninja, Cutter, x64dbg, etc. These plugins connect to the `/REToolSync` endpoint and respond to information requests, based on a _context_.

## Payloads

TODO: write about general payload structure (JSON), how things relate to the context.

### Cursor

TODO: Document `cursor_request.json` and `cursor_response.json`