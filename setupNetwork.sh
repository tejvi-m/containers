BRIDGE = "containerbr0"
ETH0 = "virbr0"

sudo iptables -A FORWARD -i $ETH0 -o veth -j ACCEPT || true
	sudo iptables -A FORWARD -o $ETH0 -i veth -j ACCEPT || true
	sudo iptables -t nat -A POSTROUTING -s 172.16.0.0/16 -j MASQUERADE || true
	sudo ip link add name $BRIDGE type bridge || true
	sudo ip addr add dev $BRIDGE 172.16.0.1/24 || true
	sudo ip link set $BRIDGE up || true
	sudo iptables -A FORWARD -i $ETH0 -o $BRIDGE -j ACCEPT || true
	sudo iptables -A FORWARD -o $ETH0 -i $BRIDGE -j ACCEPT || true
	sudo iptables -A FORWARD -o $BRIDGE -i $BRIDGE -j ACCEPT || true
