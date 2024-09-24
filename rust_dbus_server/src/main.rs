
use dbus::blocking::Connection;
use dbus_crossroads::{Crossroads, Context};
use std::error::Error;

struct Hello { called_count: u32 }


fn main() -> Result<(), Box<dyn Error>> {
    let c = Connection::new_session()?;
    c.request_name("org.example.TestServer", false, true, false)?;

    let mut cr = Crossroads::new();

    let iface_token = cr.register("org.example.TestInterface", |b| {
        let message_received = b.signal::<(String,), _>("OnEmitSignal", ("sender",)).msg_fn();

        b.method("ble_message", ("name",), ("reply",), move |ctx: &mut Context, hello: &mut Hello, (name,): (String,)| {

            println!("Incoming message call from {}!", name);
            hello.called_count += 1;
            let reply = format!("This is the reply from the server");

            let signal_msg = message_received(ctx.path(), &(name,));
            ctx.push_msg(signal_msg);

            Ok((reply,))
        });
    });

    cr.insert("/org/example/TestObject", &[iface_token], Hello { called_count: 0});

    cr.serve(&c)?;
    unreachable!()
}