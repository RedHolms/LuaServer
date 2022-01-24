print(socket)
for k, v in pairs(socket) do
	print(k, v)
end
main_socket = socket.create()
for k, v in pairs(main_socket) do
	print(k, v)
end
main_socket:bind("192.168.1.106", 80)
print('binded')
main_socket:listen()
print('listening...')
conn_socket = main_socket:accept()
print('accepted!!!')
data = conn_socket:recv()
print('received data')
print(data)