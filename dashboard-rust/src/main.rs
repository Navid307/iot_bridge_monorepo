use log::*;
use std::str;
use tokio::process::Command;

#[tokio::main]
async fn main() {
    if std::env::var("RUST_LOG").is_err() {
        std::env::set_var("RUST_LOG", "info,dashboard_rust=debug");
    }

    env_logger::init();

    info!("Starting application");

    let git_version_task = tokio::spawn(async {
        match get_git_version().await {
            Ok(version) => info!("Git version: {}", version),
            Err(e) => error!("Failed to get Git version: {}", e),
        }
    });

    let _ = tokio::join!(git_version_task);
}

async fn get_git_version() -> Result<String, Box<dyn std::error::Error>> {
    let output = Command::new("git")
        .arg("log")
        .arg("-1")
        .arg("--oneline")
        .output()
        .await?; // TODO: Replace this with --describe after the first tag
    if output.status.success() {
        Ok(str::from_utf8(&output.stdout)?.trim().to_string())
    } else {
        Err("Git command failed".into())
    }
}
