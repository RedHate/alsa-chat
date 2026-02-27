
achat with opus 4kHz encoding / decoding

 Probably not the best way to create a voice chat, i pretty much broke all the rules lol
 Still need to convert tcp stuff to udp but is in a decent working state.

	Usage:
		Server:
			./achat [listener_port] [capture device] [playback device]

		Client:
			./achat [server_ip] [port] [capture device] [playback device]

		Example:
			Server:
				./achat 1122 default default

			Client:
				./achat 127.0.0.1 1122 default default
