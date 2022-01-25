main_socket = socket.create()
main_socket:bind("192.168.1.106", 80)
main_socket:listen()
while true do 
	print('listening...')
	conn_socket = main_socket:accept()
	print("connected")
	data = conn_socket:recv()
	print(data)
	conn_socket:send("It's Working!!!")
	conn_socket:close()
end
