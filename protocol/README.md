# Protocol

This directory contains documentation and example JSON payloads for the REToolSync protocol.

## Architecture

The protocol has been designed to make it as simple as possible to implement REToolSync clients. `REToolSyncServer` is a central server (optionally hosted on the public internet). Its only job is to broadcast any message sent to the `/REToolSync` endpoint back to all clients, except the client sending the initial message.

The functionality of `REToolSync` is implemented by _intelligence providers_, usually plugins for reverse engineering tools like IDA, Ghidra, Binary Ninja, Cutter, x64dbg, etc. These plugins connect to the `/REToolSync` endpoint and respond to information requests, based on a _context_.

## Debugging the communication

Because the design is based on a single websocket channel it's quite easy to debug what's going on with any simple websocket test service.

- [websocket echo test](https://www.websocket.org/echo.html)

- [Simple WebSocket Client](https://chrome.google.com/webstore/detail/simple-websocket-client/pfdhoblngboilpfeibdedpjgfnlcodoo?hl=en) (Chrome)

Perhaps at a later date a better custom debugging tool for the protocol will be added.

## Debugging on the public internet

You can use [localtunnel](https://theboroer.github.io/localtunnel-www/) to expose `REToolSyncServer` on the public internet. You can then use and share the link to connect to your own private instance.

First install `localtunnel`:

```sh
> npm install -g localtunnel
```

Then start `REToolSyncServer` :

```sh
> REToolSyncServer
Listening on 127.0.0.1:6969/REToolSync..
```

Finally expose the server to the public internet:

```sh
> lt --port 6969
your url is: https://xxx-yyy-123.loca.lt
```

You should now be able to connect to both `ws://xxx-yyy-123.loca.lt/REToolSync` (insecure) and `wss://xxx-yyy-123.loca.lt/REToolSync`.

## Payloads

TODO: write about general payload structure (JSON), how things relate to the context.

### Cursor

TODO: Document `cursor_request.json` and `cursor_response.json`