#!/usr/bin/env python3
"""
Simple local HTTP server to launch the Stadia-Pico-PS5 Test & Remap Studio.
Run with: python web/serve.py
"""
import http.server
import socketserver
import webbrowser
import os
import sys

PORT = 8080
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

def main():
    print("=" * 60)
    print("  Stadia to PS5 / Brook Wingman P5 - Test & Remap Studio")
    print(f"  Serving directory: {DIRECTORY}")
    print(f"  Opening URL: http://localhost:{PORT}")
    print("=" * 60)

    try:
        with socketserver.TCPServer(("", PORT), Handler) as httpd:
            webbrowser.open(f"http://localhost:{PORT}")
            print(f"Server running at http://localhost:{PORT} (Press Ctrl+C to stop)")
            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
    except Exception as e:
        print(f"Error starting server: {e}")
        print(f"You can also open '{os.path.join(DIRECTORY, 'index.html')}' directly in your browser.")

if __name__ == "__main__":
    main()
