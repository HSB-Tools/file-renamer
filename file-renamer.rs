// main.rs

use std::env;
use std::fs;
use std::io::{self, Write};
use std::path::PathBuf;
use std::process;

fn main() {
    // Collect all arguments from the command line
    let args: Vec<String> = env::args().collect();

    // Initialize variables to store flag values
    let mut path_str: Option<String> = None;
    let mut from_ext_str: Option<String> = None;
    let mut to_ext_str: Option<String> = None;
    let mut skip_confirmation = false;

    // Iterate through arguments to find flags
    let mut i = 1; // Start at index 1 as args[0] is the program name
    while i < args.len() {
        match args[i].as_str() {
            "-h" | "--help" => {
                print_usage_and_exit("");
            }
            "-p" | "--path" => {
                i += 1;
                if i < args.len() {
                    path_str = Some(args[i].clone());
                } else {
                    print_usage_and_exit("Error: Flag -p or --path requires an argument.");
                }
            }
            "-f" | "--from" => {
                i += 1;
                if i < args.len() {
                    from_ext_str = Some(args[i].clone());
                } else {
                    print_usage_and_exit("Error: Flag -f or --from requires an argument.");
                }
            }
            "-t" | "--to" => {
                i += 1;
                if i < args.len() {
                    to_ext_str = Some(args[i].clone());
                } else {
                    print_usage_and_exit("Error: Flag -t or --to requires an argument.");
                }
            }
            "-y" => {
                skip_confirmation = true;
            }
            _ => print_usage_and_exit(&format!("Error: Unknown argument '{}'.", args[i])),
        }
        i += 1;
    }

    // Validate and unwrap arguments explicitly
    if path_str.is_none() {
        print_usage_and_exit("Error: Path (-p) must be provided.");
    }
    let path_raw = path_str.unwrap();

    if from_ext_str.is_none() {
        print_usage_and_exit("Error: 'From' extension (-f) must be provided.");
    }
    let from_ext_raw = from_ext_str.unwrap();

    if to_ext_str.is_none() {
        print_usage_and_exit("Error: 'To' extension (-t) must be provided.");
    }
    let to_ext_raw = to_ext_str.unwrap();


    // Clean the extension by removing a leading '.', if any.
    let from_ext = from_ext_raw.strip_prefix('.').unwrap_or(&from_ext_raw).to_string();
    let to_ext = to_ext_raw.strip_prefix('.').unwrap_or(&to_ext_raw).to_string();

    let path = PathBuf::from(&path_raw);

    if !path.is_dir() {
        eprintln!("Error: The provided path is not a valid directory: {}", path.display());
        process::exit(1);
    }

    println!("Scanning directory: {}", path.display());

    match fs::read_dir(&path) {
        Ok(entries) => {
            let mut files_to_rename = Vec::new();
            for entry in entries {
                if let Ok(entry) = entry {
                    let current_path = entry.path();
                    if current_path.is_file() {
                        if let Some(extension) = current_path.extension().and_then(|s| s.to_str()) {
                            if extension.eq_ignore_ascii_case(&from_ext) {
                                files_to_rename.push(current_path);
                            }
                        }
                    }
                }
            }

            if files_to_rename.is_empty() {
                println!("No files with extension '.{}' found to rename.", from_ext);
                process::exit(0);
            }

            println!("file found:");
            for file_path in &files_to_rename {
                println!("  {}", file_path.display());
            }
             println!("Will change extensions from '.{}' to '.{}'", from_ext, to_ext);


            if !skip_confirmation {
                print!("Do you want to proceed with renaming? (y/N): ");
                io::stdout().flush().unwrap();
                let mut input = String::new();
                io::stdin().read_line(&mut input).unwrap();
                if input.trim().to_lowercase() != "y" {
                    println!("Operation cancelled.");
                    process::exit(0);
                }
            }
            
            println!("---");

            let mut file_renamed_count = 0;
            for current_path in files_to_rename {
                let new_path = current_path.with_extension(&to_ext);
                
                println!("Renaming: {} -> {}", current_path.display(), new_path.display());

                if let Err(e) = fs::rename(&current_path, &new_path) {
                    eprintln!("  -> Failed to rename file '{}': {}", current_path.display(), e);
                } else {
                    file_renamed_count += 1;
                }
            }
            
            println!("---");
            if file_renamed_count == 0 {
                println!("No files were renamed.");
            } else if file_renamed_count == 1 {
                println!("Done. Successfully renamed 1 file.");
            } else {
                println!("Done. Successfully renamed {} files.", file_renamed_count);
            }
        }
        Err(e) => {
            eprintln!("Error: Failed to read directory '{}': {}", path.display(), e);
            process::exit(1);
        }
    }
}

fn print_usage_and_exit(message: &str) {
    if !message.is_empty() {
        eprintln!("{}", message);
    }
    eprintln!("\nUsage:");
    eprintln!("  ./file_renamer -p <directory_path> -f <from_extension> -t <to_extension> [-y]");
    eprintln!("\nArguments:");
    eprintln!("  -p, --path <path>      : Sets the directory path to scan");
    eprintln!("  -f, --from <ext>       : Sets the extension to rename from");
    eprintln!("  -t, --to <ext>         : Sets the extension to rename to");
    eprintln!("  -y                     : Skips the confirmation prompt");
    eprintln!("  -h, --help             : Displays this help message");
    eprintln!("\nExample:");
    eprintln!("  ./file_renamer -p /home/user/documents -f cpp -t txt");
    process::exit(1);
}
