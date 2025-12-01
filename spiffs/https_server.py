import http.server, ssl

server_address = ('0.0.0.0', 8000)
httpd = http.server.HTTPServer(server_address,http.server.SimpleHTTPRequestHandler)

context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
context.load_cert_chain(certfile="server_cert.pem",keyfile="server_key.pem")
httpd.socket = context.wrap_socket(httpd.socket, server_side=True)



print("server HTTPS on port 8000....")

httpd.serve_forever()