import tkinter as tk
from tkinter import messagebox, ttk
import socket
import threading
from typing import Optional, Dict
import re # For parsing the tree output

# --- Configuration ---
SERVER_IP = "127.00.0.1"
SERVER_PORT = 19090
BUFFER_SIZE = 8192
# ---------------------

class OFSClientGUI:
    def __init__(self, master: tk.Tk):
        self.master = master
        master.title("🌐 Omni File System Client")
        master.geometry("1000x700") 

        self.sock: Optional[socket.socket] = None
        self.is_connected = False
        self.command_buttons = []
        self.last_inserted_dir_id = '' # Tracker for flat output fallback

        self.setup_ui()
        self.master.protocol("WM_DELETE_WINDOW", self.close_client)

    def setup_ui(self):
        # 0. Connection Status Frame
        status_frame = tk.Frame(self.master, bd=1, relief=tk.SUNKEN, padx=5, pady=5)
        status_frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=5)
        
        self.status_label = tk.Label(status_frame, text="Status: Disconnected", fg="red", font=('Arial', 10, 'bold'))
        self.status_label.pack(side=tk.LEFT, padx=5)
        
        self.connect_button = tk.Button(status_frame, text="🔌 Connect to Server", command=self.connect_to_server)
        self.connect_button.pack(side=tk.RIGHT, padx=5)

        # 1. Output Panels Frame (Treeview and Command Log)
        output_panels_frame = tk.Frame(self.master)
        output_panels_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=5)
        output_panels_frame.grid_columnconfigure(0, weight=2) 
        output_panels_frame.grid_columnconfigure(1, weight=1) 
        
        # 1A. Treeview Frame (File Explorer)
        tree_frame = tk.LabelFrame(output_panels_frame, text="File Explorer (Current User)", padx=5, pady=5)
        tree_frame.grid(row=0, column=0, sticky='nsew', padx=5, pady=5)

        style = ttk.Style()
        style.configure("Treeview", rowheight=25)
        style.configure("Treeview.Heading", font=('Arial', 10, 'bold'))

        self.tree = ttk.Treeview(tree_frame, columns=('type', 'path'), show='tree headings')
        self.tree.heading('#0', text='Name', anchor='w')
        self.tree.column('#0', width=300, anchor='w')
        
        self.tree.column('type', width=0, stretch=tk.NO)
        self.tree.column('path', width=0, stretch=tk.NO)

        self.tree.bind('<<TreeviewSelect>>', self._on_tree_select)
        self.tree.bind('<Double-1>', self._on_double_click) 
        
        # Scrollbar for Treeview
        tree_scroll = ttk.Scrollbar(tree_frame, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=tree_scroll.set)
        
        tree_scroll.pack(side='right', fill='y')
        self.tree.pack(side='left', fill='both', expand=True)

        # 1B. Command Log Frame
        log_frame = tk.LabelFrame(output_panels_frame, text="Command Log", padx=5, pady=5)
        log_frame.grid(row=0, column=1, sticky='nsew', padx=5, pady=5)

        self.output_log = tk.Text(log_frame, height=15, state=tk.DISABLED, bg="#f0f0f0")
        self.output_log.tag_config('green', foreground='green')
        self.output_log.tag_config('red', foreground='red')
        self.output_log.tag_config('blue', foreground='blue')
        self.output_log.tag_config('gray', foreground='gray') 
        
        self.output_log.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # 2. Controls Frame 
        controls_frame = tk.Frame(self.master, padx=10, pady=10)
        controls_frame.pack(side=tk.BOTTOM, fill=tk.X)
        
        controls_frame.grid_columnconfigure(0, weight=1)
        controls_frame.grid_columnconfigure(1, weight=1)

        # --- A. System/Auth Controls ---
        sys_auth_frame = tk.LabelFrame(controls_frame, text="System & Authentication", padx=5, pady=5)
        sys_auth_frame.grid(row=0, column=0, sticky='nsew', padx=5, pady=5)
        
        self.command_buttons.append(tk.Button(sys_auth_frame, text="1. FORMAT", command=lambda: self.send_command("FORMAT")))
        self.command_buttons.append(tk.Button(sys_auth_frame, text="2. LOAD OFS", command=lambda: self.send_command("LOAD")))
        self.command_buttons.append(tk.Button(sys_auth_frame, text="3. Create User", command=self.show_create_user_dialog))
        self.command_buttons.append(tk.Button(sys_auth_frame, text="4. Login", command=self.show_login_dialog))
        self.command_buttons.append(tk.Button(sys_auth_frame, text="5. Logout", command=lambda: self.send_command("LOGOUT")))
        self.command_buttons.append(tk.Button(sys_auth_frame, text="6. Delete User", command=self.show_delete_user_dialog))

        for btn in self.command_buttons[-6:]: btn.pack(fill='x', pady=2)
        
        # --- B. File/Dir Controls ---
        file_dir_frame = tk.LabelFrame(controls_frame, text="File & Directory Operations", padx=5, pady=5)
        file_dir_frame.grid(row=0, column=1, sticky='nsew', padx=5, pady=5)
        
        self.command_buttons.append(tk.Button(file_dir_frame, text="16. Create Dir", command=lambda: self.show_path_dialog("CREATE_DIR", "Directory Path")))
        self.command_buttons.append(tk.Button(file_dir_frame, text="17. Create File", command=self.show_create_file_dialog))
        self.command_buttons.append(tk.Button(file_dir_frame, text="19. Delete File", command=lambda: self.show_path_dialog("DELETE_FILE", "File Path")))
        self.command_buttons.append(tk.Button(file_dir_frame, text="20. Delete Dir", command=lambda: self.show_path_dialog("DELETE_DIR", "Directory Path")))
        self.command_buttons.append(tk.Button(file_dir_frame, text="21. Write/Overwrite File", command=self.show_write_file_dialog))

        for btn in self.command_buttons[-5:]: btn.pack(fill='x', pady=2)


        # --- C. Info/List Controls (Below File/Dir) ---
        info_list_frame = tk.LabelFrame(controls_frame, text="Information & Listing", padx=5, pady=5)
        info_list_frame.grid(row=1, column=0, columnspan=2, sticky='ew', padx=5, pady=5)
        
        inner_info_frame = tk.Frame(info_list_frame)
        inner_info_frame.pack(fill='x')
        
        self.command_buttons.append(tk.Button(inner_info_frame, text="22. Truncate Size (Advanced)", command=self.show_truncate_dialog))
        self.command_buttons.append(tk.Button(inner_info_frame, text="7. Read Block", command=lambda: self.show_path_dialog("READ_BLOCK", "Block Index")))
        self.command_buttons.append(tk.Button(inner_info_frame, text="8. STATS", command=lambda: self.send_command("STATS")))
        self.command_buttons.append(tk.Button(inner_info_frame, text="9. Change Log", command=lambda: self.send_command("SHOW_CHANGE_LOG")))

        for btn in self.command_buttons[-4:]: btn.pack(side=tk.LEFT, expand=True, fill='x', padx=2, pady=2)


        inner_info_frame_2 = tk.Frame(info_list_frame)
        inner_info_frame_2.pack(fill='x')
        
        self.command_buttons.append(tk.Button(inner_info_frame_2, text="13. List Users", command=lambda: self.send_command("LIST_USERS")))
        self.command_buttons.append(tk.Button(inner_info_frame_2, text="14. List MY Files", command=lambda: self.send_command("LIST_MY_FILES")))
        self.command_buttons.append(tk.Button(inner_info_frame_2, text="15. List ALL Files", command=lambda: self.send_command("LIST_ALL_FILES")))
        self.command_buttons.append(tk.Button(inner_info_frame_2, text="18. Show Tree (Refresh Explorer)", command=self.do_show_tree))
        
        for btn in self.command_buttons[-4:]: btn.pack(side=tk.LEFT, expand=True, fill='x', padx=2, pady=2)
        
        # --- D. Initial state and Quit Button ---
        tk.Button(controls_frame, text="0. QUIT", command=self.close_client, bg='red', fg='white').grid(row=2, column=0, columnspan=2, sticky='ew', padx=5, pady=10)
        
        self._set_button_state(tk.DISABLED)

    def _set_button_state(self, state):
        """Enables or disables all command buttons."""
        for btn in self.command_buttons:
            btn.config(state=state)

    def log_output(self, message: str, color: str = 'black'):
        """Writes a message to the output text area, optionally with color."""
        self.output_log.config(state=tk.NORMAL)
        
        tags = (color,) if color in ('green', 'red', 'blue', 'gray') else ()
        
        self.output_log.insert(tk.END, message + '\n', tags)
        self.output_log.see(tk.END)
        self.output_log.config(state=tk.DISABLED)

    def _on_tree_select(self, event):
        """Handles item selection in the Treeview (File Explorer)."""
        selected_item = self.tree.selection()
        if not selected_item:
            return

        item_id = selected_item[0]
        item_data = self.tree.item(item_id, 'values')
        
        if not item_data:
            return

        item_type = item_data[0] 
        item_path = item_data[1] 
        
        self.log_output(f"🔍 Selected {item_type}: {item_path}", 'blue')

    def _on_double_click(self, event):
        """Handles double-click events, reading file content if a file is selected."""
        item_id = self.tree.identify_row(event.y)
        if not item_id:
            return
            
        item_data = self.tree.item(item_id, 'values')
        if not item_data:
            return

        item_type = item_data[0] 
        item_path = item_data[1] 

        if item_type == 'File':
            self.log_output(f"📄 Double-click detected. Reading content of: {item_path}", 'blue')
            cmd = f"READ_FILE|{item_path}" 
            threading.Thread(target=self._execute_read_file_command, args=(cmd, item_path)).start()

    def _execute_read_file_command(self, cmd, path):
        """Executes the READ_FILE command and shows the content in a dialog."""
        try:
            msg = cmd + "\n"
            self.sock.sendall(msg.encode('utf-8'))
            data = self.sock.recv(BUFFER_SIZE)
            response = data.decode('utf-8').strip()

            if response.startswith("ERR|"):
                self.master.after(0, self.log_output, f"🔴 ERROR reading {path}: {response.split('|', 1)[-1]}", 'red')
            else:
                self.master.after(0, self.show_read_file_content_dialog, path, response)
        
        except socket.error as e:
            self.master.after(0, self._on_connection_failure, e)
            
    def show_read_file_content_dialog(self, path, content):
        """Shows the content of a file in a modal dialog."""
        dialog = tk.Toplevel(self.master)
        dialog.title(f"File Content: {path}")
        dialog.transient(self.master)
        dialog.grab_set()
        
        tk.Label(dialog, text=f"Content of {path}:", font=('Arial', 10, 'bold')).pack(padx=10, pady=5, anchor='w')
        
        content_text = tk.Text(dialog, height=15, width=60, wrap=tk.WORD, bg='#f0f0f0')
        content_text.insert(tk.END, content)
        content_text.config(state=tk.DISABLED) 
        content_text.pack(padx=10, pady=5)
        
        tk.Button(dialog, text="Close", command=dialog.destroy).pack(pady=10)

    def _parse_and_display_tree(self, tree_data: str):
        """
        Parses the server's indented tree output and populates the Treeview.
        This includes a robust fallback for non-indented (flat) server output.
        """
        self.tree.delete(*self.tree.get_children())
        
        lines = tree_data.split('\n')
        # Dictionary to track parent item IDs by indentation level (column index of name start)
        parent_map: Dict[int, str] = {-4: ''} # Use a negative index for the root 
        
        # Reset tracker for flat output fallback
        self.last_inserted_dir_id = '' 

        for line in lines:
            if not line.strip():
                continue

            # --- 1. Determine Logical Indentation (Level) ---
            # Find the index where the actual name starts (ignoring spaces/pipes/dashes/etc.)
            # This is key for indented output (e.g., from admin)
            match_name_start = re.search(r'[^ \t|`+\-\\]', line)
            
            if not match_name_start:
                continue

            name_start_index = match_name_start.start()
            level_key = name_start_index

            # --- 2. Extract the Name ---
            name = line[name_start_index:].strip()
            
            # --- Filter/Header Check ---
            # Skip informational lines that aren't part of the structure
            if 'structure for user:' in name or 'Files for user' in name or not name:
                self.log_output(f"Parser skipping informational/empty line: {name}", 'gray')
                continue
            
            # --- Type Heuristic ---
            item_type = 'Dir'
            # A file typically has a '.' and does not end with '/'
            if '.' in name and not name.endswith('/'):
                 item_type = 'File'
            
            # Ensure directory names have a trailing slash for path construction accuracy
            if item_type == 'Dir' and not name.endswith('/'):
                 name += '/' 

            # 3. Determine Parent ID and Full Path
            
            parent_id = '' # Default to root

            # Standard Parent Determination (for indented output)
            parent_key = -4
            for k in sorted(parent_map.keys(), reverse=True):
                if k < level_key:
                    parent_key = k
                    break
            
            parent_id = parent_map.get(parent_key, '')
            
            # *** ROBUST FIX FOR FLAT SERVER OUTPUT (Level 0 Indentation Failure) ***
            # If the current item is at level 0, and we have a stored parent directory (from a previous level 0 item),
            # we force it to be a child of that stored parent.
            if level_key == 0 and self.last_inserted_dir_id:
                parent_id = self.last_inserted_dir_id
            # *** END FIX ***
            
            # Get the full path of the parent item 
            parent_path_values = self.tree.item(parent_id, 'values')
            parent_path_value = parent_path_values[1] if parent_path_values and len(parent_path_values) > 1 else ''
            
            # Construct the full path
            if parent_path_value:
                # Remove leading slashes from the name before combining, then replace any double slashes
                item_name_to_append = name.lstrip('/')
                # If the parent path is just '/' or empty, we don't want a double slash
                full_path = f"{parent_path_value}{item_name_to_append}".replace('//', '/')
                # Ensure path starts with a single / if it's not the root itself
                if not full_path.startswith('/'):
                    full_path = '/' + full_path
            else:
                # This is a top-level root item (e.g., 'admin/' or 'user1/')
                full_path = name.lstrip('/')
                if not full_path.startswith('/'):
                    full_path = '/' + full_path

            # 4. Insert the item
            display_name = name.rstrip('/')
            
            item_id = self.tree.insert(
                parent_id, 
                'end', 
                text=display_name, 
                values=(item_type, full_path),
                open=(item_type == 'Dir')
            )
            
            # 5. Update parent map and flat view tracker
            parent_map[level_key] = item_id
            
            # The flat view fallback MUST track the parent directory's ID if one was inserted.
            if item_type == 'Dir':
                self.last_inserted_dir_id = item_id
            
            # If the indentation returns to the root (level 0) and the previous item was a directory, 
            # we need to pop the stored directory to allow the next item to potentially be another root.
            # However, since we're mostly dealing with single-user trees, the simple `if level_key == 0 and self.last_inserted_dir_id:` 
            # above should be enough to force nesting. The debug log will help confirm.

            # --- DEBUG LOGGING ---
            self.log_output(f"   [DBG] Name: '{display_name}' | Path: '{full_path}' | Level: {level_key} | Parent ID: {parent_id[:8]}...", 'gray')
            # ---------------------

    def do_show_tree(self):
        """Sends the SHOW_TREE command and processes the result for the Treeview."""
        if not self.is_connected or not self.sock:
            self.log_output("❌ Not connected to server.", 'red')
            return
        
        self.log_output("> Sending: SHOW_TREE for explorer update")
        
        threading.Thread(target=self._execute_tree_command).start()

    def _execute_tree_command(self):
        """The actual network communication for SHOW_TREE."""
        cmd = "SHOW_TREE"
        try:
            self.sock.sendall((cmd + "\n").encode('utf-8'))
            data = self.sock.recv(BUFFER_SIZE)
            response = data.decode('utf-8').strip()

            if response.startswith("ERR|"):
                self.master.after(0, self.log_output, f"🔴 ERROR showing tree: {response.split('|', 1)[-1]}", 'red')
            else:
                self.master.after(0, self.log_output, "🟢 Tree structure received. Updating explorer.", 'green')
                self.master.after(0, self._parse_and_display_tree, response)

        except socket.error as e:
            self.master.after(0, self._on_connection_failure, e)


    # --- Connection Methods ---
    def connect_to_server(self):
        """Attempts to connect to the server in a separate thread."""
        if self.is_connected:
            self.log_output("⚠️ Already connected.", 'blue')
            return
            
        self.status_label.config(text="Status: Connecting...", fg="orange")
        self.connect_button.config(state=tk.DISABLED)

        threading.Thread(target=self._attempt_connection).start()

    def _attempt_connection(self):
        """The actual connection logic."""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((SERVER_IP, SERVER_PORT))
            self.is_connected = True
            
            self.master.after(0, self._on_connection_success)
            
        except socket.error as e:
            self.master.after(0, self._on_connection_failure, e)
            
    def _on_connection_success(self):
        """Updates GUI after a successful connection."""
        self.log_output(f"✅ Connected to OFS Server at {SERVER_IP}:{SERVER_PORT}", 'green')
        self.status_label.config(text="Status: Connected", fg="green")
        self.connect_button.config(text="🔌 Disconnect", command=self.disconnect_from_server, state=tk.NORMAL)
        self._set_button_state(tk.NORMAL)

    def _on_connection_failure(self, error):
        """Updates GUI after a failed connection."""
        self.log_output(f"❌ Connection Failed: {error}", 'red')
        self.status_label.config(text="Status: Disconnected", fg="red")
        self.connect_button.config(text="🔌 Connect to Server", command=self.connect_to_server, state=tk.NORMAL)
        self.sock = None
        self.is_connected = False
        self._set_button_state(tk.DISABLED)

    def disconnect_from_server(self):
        """Disconnects the client and updates the GUI state."""
        self._set_button_state(tk.DISABLED)
        self.status_label.config(text="Status: Disconnecting...", fg="orange")
        self.connect_button.config(state=tk.DISABLED)
        
        if self.sock:
            try:
                self.sock.sendall(b"QUIT\n")
                self.sock.close()
            except socket.error:
                pass
        
        self.sock = None
        self.is_connected = False
        self.status_label.config(text="Status: Disconnected", fg="red")
        self.connect_button.config(text="🔌 Connect to Server", command=self.connect_to_server, state=tk.NORMAL)
        self.log_output("Connection closed successfully.", 'blue')
        

    def send_command(self, cmd: str):
        """Sends a command string to the server in a worker thread."""
        if not self.is_connected or not self.sock:
            self.log_output("❌ Not connected to server. Press 'Connect to Server' first.", 'red')
            return
        
        self.log_output(f"> Sending: {cmd}")
        
        threading.Thread(target=self._execute_command, args=(cmd,)).start()

    def _execute_command(self, cmd: str):
        """The actual network communication executed in a thread."""
        try:
            msg = cmd + "\n"
            self.sock.sendall(msg.encode('utf-8'))

            data = self.sock.recv(BUFFER_SIZE)
            response = data.decode('utf-8').strip()
            
            upper_response = response.upper()

            if response.startswith("OK|"):
                self.log_output(f"🟢 OK: {response.split('|', 1)[-1]}", 'green')
            elif response.startswith("ERR|"):
                self.log_output(f"🔴 ERROR: {response.split('|', 1)[-1]}", 'red')
            elif "ACCESS DENIED" in upper_response or "PERMISSION DENIED" in upper_response or "NOT AUTHORIZED" in upper_response:
                 self.log_output(f"🔴 ERROR (Access/Permission Issue Detected): {response}", 'red')
            else:
                self.log_output(response) 
        
        except socket.error as e:
            self.master.after(0, self._on_connection_failure, e)

    def close_client(self):
        """Handles closing the GUI and disconnecting."""
        if self.is_connected:
            self.disconnect_from_server()
        self.master.quit()

    # --- Dialog Methods (omitted for brevity, they are unchanged) ---
    def show_login_dialog(self):
        dialog = tk.Toplevel(self.master)
        dialog.title("Login")
        
        tk.Label(dialog, text="Username:").grid(row=0, column=0, padx=5, pady=5)
        user_entry = tk.Entry(dialog)
        user_entry.grid(row=0, column=1, padx=5, pady=5)
        
        tk.Label(dialog, text="Password:").grid(row=1, column=0, padx=5, pady=5)
        pass_entry = tk.Entry(dialog, show='*')
        pass_entry.grid(row=1, column=1, padx=5, pady=5)
        
        def do_login():
            user = user_entry.get()
            password = pass_entry.get()
            if user and password:
                self.send_command(f"LOGIN|{user}|{password}")
                dialog.destroy()
            else:
                messagebox.showerror("Error", "Both fields are required.")

        tk.Button(dialog, text="Login", command=do_login).grid(row=2, column=0, columnspan=2, pady=10)

    def show_create_user_dialog(self):
        dialog = tk.Toplevel(self.master)
        dialog.title("Create New User")
        
        tk.Label(dialog, text="Username:").grid(row=0, column=0, padx=5, pady=5)
        user_entry = tk.Entry(dialog)
        user_entry.grid(row=0, column=1, padx=5, pady=5)
        
        tk.Label(dialog, text="Password:").grid(row=1, column=0, padx=5, pady=5)
        pass_entry = tk.Entry(dialog, show='*')
        pass_entry.grid(row=1, column=1, padx=5, pady=5)

        tk.Label(dialog, text="Admin (1/0):").grid(row=2, column=0, padx=5, pady=5)
        admin_var = tk.StringVar(value='0')
        admin_entry = tk.Entry(dialog, textvariable=admin_var)
        admin_entry.grid(row=2, column=1, padx=5, pady=5)
        
        def do_create():
            user = user_entry.get()
            password = pass_entry.get()
            admin = admin_entry.get()
            if user and password and admin in ('0', '1'):
                self.send_command(f"CREATE_USER|{user}|{password}|{admin}")
                dialog.destroy()
            else:
                messagebox.showerror("Error", "All fields must be valid (Admin must be 1 or 0).")

        tk.Button(dialog, text="Create User", command=do_create).grid(row=3, column=0, columnspan=2, pady=10)
    
    def show_delete_user_dialog(self):
        dialog = tk.Toplevel(self.master)
        dialog.title("Delete User")
        
        tk.Label(dialog, text="Username to Delete:").grid(row=0, column=0, padx=5, pady=5)
        user_entry = tk.Entry(dialog, width=40)
        user_entry.grid(row=0, column=1, padx=5, pady=5)
        
        def do_delete():
            user = user_entry.get()
            if user:
                self.send_command(f"DELETE_USER|{user}")
                dialog.destroy()
            else:
                messagebox.showerror("Error", "Username is required.")

        tk.Button(dialog, text="Delete User", command=do_delete).grid(row=1, column=0, columnspan=2, pady=10)

    def show_path_dialog(self, command_name, prompt):
        dialog = tk.Toplevel(self.master)
        dialog.title(f"{command_name.replace('_', ' ')}")
        
        tk.Label(dialog, text=f"{prompt}:").grid(row=0, column=0, padx=5, pady=5)
        path_entry = tk.Entry(dialog, width=40)
        path_entry.grid(row=0, column=1, padx=5, pady=5)
        
        def do_command():
            path = path_entry.get()
            if path:
                self.send_command(f"{command_name}|{path}")
                dialog.destroy()
            else:
                messagebox.showerror("Error", f"{prompt} is required.")

        tk.Button(dialog, text=f"Execute {command_name}", command=do_command).grid(row=1, column=0, columnspan=2, pady=10)

    def show_create_file_dialog(self):
        dialog = tk.Toplevel(self.master)
        dialog.title("Create File")
        
        tk.Label(dialog, text="File Path:").grid(row=0, column=0, padx=5, pady=5, sticky='w')
        path_entry = tk.Entry(dialog, width=50)
        path_entry.grid(row=0, column=1, padx=5, pady=5, sticky='ew')
        
        tk.Label(dialog, text="Content:").grid(row=1, column=0, padx=5, pady=5, sticky='w')
        content_entry = tk.Text(dialog, height=5, width=50)
        content_entry.grid(row=1, column=1, padx=5, pady=5, sticky='ew')
        
        def do_create_file():
            path = path_entry.get()
            content = content_entry.get("1.0", tk.END).strip()
            
            if path and content:
                self.send_command(f"CREATE_FILE|{path}|{content}")
                dialog.destroy()
            else:
                messagebox.showerror("Error", "Path and Content are required.")

        tk.Button(dialog, text="Create File", command=do_create_file).grid(row=2, column=0, columnspan=2, pady=10)

    def show_write_file_dialog(self):
        dialog = tk.Toplevel(self.master)
        dialog.title("Write/Overwrite File Content")
        
        tk.Label(dialog, text="File Path:").grid(row=0, column=0, padx=5, pady=5, sticky='w')
        path_entry = tk.Entry(dialog, width=50)
        path_entry.grid(row=0, column=1, padx=5, pady=5, sticky='ew')
        
        tk.Label(dialog, text="New Content:").grid(row=1, column=0, padx=5, pady=5, sticky='w')
        content_entry = tk.Text(dialog, height=5, width=50)
        content_entry.grid(row=1, column=1, padx=5, pady=5, sticky='ew')
        
        content_entry.insert(tk.END, "dr umer suleman")

        def do_write_file():
            path = path_entry.get()
            content = content_entry.get("1.0", tk.END).strip()
            
            if path and content:
                self.send_command(f"WRITE_FILE|{path}|{content}")
                dialog.destroy()
            else:
                messagebox.showerror("Error", "Path and Content are required.")

        tk.Button(dialog, text="Write/Overwrite", command=do_write_file).grid(row=2, column=0, columnspan=2, pady=10)
        
    def show_truncate_dialog(self):
        dialog = tk.Toplevel(self.master)
        dialog.title("Truncate File (Change Size in bytes)")
        
        tk.Label(dialog, text="File Path to Truncate:").grid(row=0, column=0, padx=5, pady=5, sticky='w')
        path_entry = tk.Entry(dialog, width=40)
        path_entry.grid(row=0, column=1, padx=5, pady=5, sticky='ew')
        
        tk.Label(dialog, text="New Length (bytes):").grid(row=1, column=0, padx=5, pady=5, sticky='w')
        length_entry = tk.Entry(dialog, width=40)
        length_entry.grid(row=1, column=1, padx=5, pady=5, sticky='ew')
        
        def do_truncate():
            path = path_entry.get()
            length_str = length_entry.get()
            
            if not path:
                messagebox.showerror("Error", "File Path is required.")
                return

            try:
                length = int(length_str)
                if length < 0:
                    raise ValueError("Length cannot be negative.")
            except ValueError:
                messagebox.showerror("Error", "New Length must be a non-negative integer.")
                return

            self.send_command(f"TRUNCATE|{path}|{length}")
            dialog.destroy()

        tk.Button(dialog, text="Execute TRUNCATE", command=do_truncate).grid(row=2, column=0, columnspan=2, pady=10)


if __name__ == '__main__':
    root = tk.Tk()
    client_gui = OFSClientGUI(root)
    root.mainloop()