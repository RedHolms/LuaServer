print(socket)
for k, v in pairs(socket) do
	print(k, v)
end
main_socket = socket.create()
print(type(main_socket.socketData))
main_socket:bind()