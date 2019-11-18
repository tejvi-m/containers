sudo iptables -D FORWARD -i $(ETH0) -o veth -j ACCEPT || true
	sudo iptables -D FORWARD -o $(ETH0) -i veth -j ACCEPT || true
	sudo iptables -t nat -D POSTROUTING -s 172.16.0.0/16 -j MASQUERADE || true
	sudo iptables -D FORWARD -i $(ETH0) -o $(BRIDGE) -j ACCEPT || true
	sudo iptables -D FORWARD -o $(ETH0) -i $(BRIDGE) -j ACCEPT || true
	sudo iptables -D FORWARD -o $(BRIDGE) -i $(BRIDGE) -j ACCEPT || true
	sudo ip link delete $(BRIDGE) || true
