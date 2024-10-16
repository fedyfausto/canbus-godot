
#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

using namespace godot;

class CanBus : public Node {
	GDCLASS(CanBus, Node);

public:

private:
    bool opened; 
	int canfd;
    char * ifname;
    Ref<Thread> thread;
    int receive();

    void threaded_operation();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:

    CanBus();
    ~CanBus();

    
    bool open(Variant _ifname);
    bool is_open();
    int send(int canid, PackedByteArray buf, int len);

    void _process(double p_delta) override;
	
};