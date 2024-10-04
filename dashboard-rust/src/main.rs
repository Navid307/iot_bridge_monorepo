use futures_util::{SinkExt, StreamExt};
use std::collections::HashMap;
use std::sync::{
    atomic::{AtomicUsize, Ordering},
    Arc,
};
use tokio::sync::{mpsc, RwLock};
use tokio_stream::wrappers::UnboundedReceiverStream;
use warp::ws::{Message, WebSocket};
use warp::Filter;

// D-Bus dependencies
use zbus::{fdo::Result as FdoResult, Connection};

/// Our global unique user id counter.
static NEXT_USER_ID: AtomicUsize = AtomicUsize::new(1);

/// Our state of currently connected users.
/// Key is their id, Value is a sender of `warp::ws::Message`
type Users = Arc<RwLock<HashMap<usize, mpsc::UnboundedSender<Message>>>>;

#[tokio::main]
async fn main() {
    // WebSocket users state.
    let users = Users::default();
    let users_clone = users.clone();

    // Spawn WebSocket server thread
    let ws_handle =
        tokio::spawn(async move {
            // Turn our "state" into a warp Filter...
            let users_filter = warp::any().map(move || users_clone.clone());

            // WebSocket route (GET /chat)
            let chat = warp::path("chat").and(warp::ws()).and(users_filter).map(
                |ws: warp::ws::Ws, users| {
                    ws.on_upgrade(move |socket| user_connected(socket, users))
                },
            );

            // Index route (GET /)
            let index = warp::path::end().map(|| warp::reply::html(INDEX_HTML));

            let routes = index.or(chat);

            warp::serve(routes).run(([127, 0, 0, 1], 3030)).await;
        });

    // Spawn D-Bus listener thread
    let dbus_handle = tokio::spawn(async move {
        match run_dbus_listener(users).await {
            Ok(_) => println!("D-Bus listener finished successfully"),
            Err(e) => eprintln!("D-Bus listener failed: {}", e),
        }
    });

    // Await both handles (WebSocket and D-Bus)
    let _ = tokio::join!(ws_handle, dbus_handle);
}

// D-Bus message forwarding
async fn run_dbus_listener(users: Users) -> FdoResult<()> {
    let connection = Connection::session().await?;
    let proxy = connection
        .proxy("org.freedesktop.DBus", "/org/freedesktop/DBus")
        .await?;

    loop {
        let msg: String = proxy.call_method("ListNames", &()).await?;
        println!("Received D-Bus message: {}", msg);

        // Broadcast the D-Bus message to all WebSocket clients
        broadcast_message(&msg, &users).await;
    }
}

// Function to broadcast a message to all connected WebSocket clients
async fn broadcast_message(msg: &str, users: &Users) {
    for (&uid, tx) in users.read().await.iter() {
        if let Err(_disconnected) = tx.send(Message::text(msg.to_string())) {
            // Handle disconnected users
        }
    }
}

// WebSocket connection handling
async fn user_connected(ws: WebSocket, users: Users) {
    let my_id = NEXT_USER_ID.fetch_add(1, Ordering::Relaxed);
    eprintln!("new chat user: {}", my_id);

    let (mut user_ws_tx, mut user_ws_rx) = ws.split();

    let (tx, rx) = mpsc::unbounded_channel();
    let mut rx = UnboundedReceiverStream::new(rx);

    tokio::task::spawn(async move {
        while let Some(message) = rx.next().await {
            user_ws_tx.send(message).await.unwrap_or_else(|e| {
                eprintln!("websocket send error: {}", e);
            });
        }
    });

    users.write().await.insert(my_id, tx);

    while let Some(result) = user_ws_rx.next().await {
        let msg = match result {
            Ok(msg) => msg,
            Err(e) => {
                eprintln!("websocket error(uid={}): {}", my_id, e);
                break;
            }
        };
        user_message(my_id, msg, &users).await;
    }

    user_disconnected(my_id, &users).await;
}

async fn user_message(my_id: usize, msg: Message, users: &Users) {
    let msg = if let Ok(s) = msg.to_str() {
        s
    } else {
        return;
    };

    let new_msg = format!("<User#{}>: {}", my_id, msg);

    for (&uid, tx) in users.read().await.iter() {
        if my_id != uid {
            if let Err(_disconnected) = tx.send(Message::text(new_msg.clone())) {}
        }
    }
}

async fn user_disconnected(my_id: usize, users: &Users) {
    eprintln!("good bye user: {}", my_id);
    users.write().await.remove(&my_id);
}

static INDEX_HTML: &str = r#"<!DOCTYPE html>
<html lang="en">
<head>
    <title>Warp Chat</title>
</head>
<body>
    <h1>Warp chat</h1>
    <div id="chat">
        <p><em>Connecting...</em></p>
    </div>
    <input type="text" id="text" />
    <button type="button" id="send">Send</button>
    <script type="text/javascript">
    const chat = document.getElementById('chat');
    const text = document.getElementById('text');
    const uri = 'ws://' + location.host + '/chat';
    const ws = new WebSocket(uri);

    ws.onopen = function() {
        chat.innerHTML = '<p><em>Connected!</em></p>';
    };

    ws.onmessage = function(msg) {
        const line = document.createElement('p');
        line.innerText = msg.data;
        chat.appendChild(line);
    };

    send.onclick = function() {
        const msg = text.value;
        ws.send(msg);
        text.value = '';
        const line = document.createElement('p');
        line.innerText = '<You>: ' + msg;
        chat.appendChild(line);
    };
    </script>
</body>
</html>
"#;
