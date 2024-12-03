import http.server
import socketserver
import cgi
import os

PORT = 8000
DIRECTORY = os.path.join(os.getcwd(), "http_root")  # Set to ${PWD}/http_root/
DATA_FILE = os.path.join(DIRECTORY, "data.txt")    # File to store POST data

# Ensure the DIRECTORY exists
os.makedirs(DIRECTORY, exist_ok=True)

class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        # Override the translate_path method to set the root directory
        path = super().translate_path(path)
        relpath = os.path.relpath(path, os.getcwd())
        return os.path.join(DIRECTORY, relpath)

    def do_POST(self):
        # Parse the form data posted
        form = cgi.FieldStorage(
            fp=self.rfile,
            headers=self.headers,
            environ={'REQUEST_METHOD': 'POST'}
        )

        # Store the form data to the file
        with open(DATA_FILE, 'a') as file:
            for field in form.keys():
                field_item = form[field]
                file.write(f"{field}={field_item.value}\n")

        # Send a response back to the client
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(b"POST request received and data stored!\n")

    def do_GET(self):
        # If the request path is for the data file
        if self.path == "/data.txt":
            if os.path.exists(DATA_FILE):
                # Read the file and send the content
                with open(DATA_FILE, 'rb') as file:
                    self.send_response(200)
                    self.send_header('Content-type', 'text/plain')
                    self.end_headers()
                    self.wfile.write(file.read())
            else:
                # If the file does not exist, send a 404 response
                self.send_response(404)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                self.wfile.write(b"File not found!\n")
        else:
            # Fallback to default GET handler (serve files)
            super().do_GET()

with socketserver.TCPServer(("", PORT), CustomHTTPRequestHandler) as httpd:
    print(f"Serving HTTP on port {PORT}")
    print(f"Serving files from {DIRECTORY}")
    httpd.serve_forever()
