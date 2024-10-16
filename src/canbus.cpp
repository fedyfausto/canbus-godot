
#include "canbus.h"


void CanBus::_bind_methods() {
	ClassDB::bind_method(D_METHOD("open", "interface"), &CanBus::open);
	ClassDB::bind_method(D_METHOD("send", "canid", "buffer", "length"), &CanBus::send);

    ADD_SIGNAL(MethodInfo("on_error", PropertyInfo(Variant::STRING, "message")));
    ADD_SIGNAL(MethodInfo("on_connected"));
    ADD_SIGNAL(MethodInfo("on_message_sent", PropertyInfo(Variant::INT, "bytes_sent")));
    ADD_SIGNAL(MethodInfo("on_message_received", PropertyInfo(Variant::INT, "packet_can_id") ,PropertyInfo(Variant::PACKED_BYTE_ARRAY, "bytes_received"), PropertyInfo(Variant::INT, "packet_length")));
}

void CanBus::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY:{
			opened = false;
            canfd = -1;
            set_process(opened);
            
            thread.instantiate();
			break;
        }
        case NOTIFICATION_EXIT_TREE: {
            opened = false;
            //if (thread.is_valid()) {
            //    thread->wait_to_finish();
            //}

            thread.unref();
            close(canfd);
        }
	}
}

CanBus::CanBus() {

}

CanBus::~CanBus() {
    set_process(false);
    close(canfd);
}

bool CanBus::is_open(){
    return (canfd == -1 ? true : false);
}

void CanBus::_process(double p_delta) {
    //if(opened)   receive();
}


bool CanBus::open(Variant _ifname){

    ifname = (char *)_ifname.stringify().utf8().ptr();
    struct sockaddr_can addr;
    struct ifreq ifr;

    /* open socket */
    if ((canfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        
        perror("socket");
        emit_signal("on_error", "socket_opening_error");
        opened = false;
        set_process(opened);
        return false;
    }
    std::cout << "connecting to " << ifname<< std::endl;



    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
    if (!ifr.ifr_ifindex) {
        perror("if_nametoindex");
        emit_signal("on_error", "interface_not_found");
        opened = false;
        set_process(opened);
        return false;
    }


    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

      /*
     * disable default receive filter on this RAW socket This is
     * obsolete as we do not read from the socket at all, but for
     * this reason we can remove the receive list in the Kernel to
     * save a little (really a very little!) CPU usage.
     */
    //setsockopt(canfd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

    if (bind(canfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        emit_signal("on_error", "bindning_Error");
        opened = false;
        set_process(opened);
        return false;
    }

    emit_signal("on_connected");
    opened = true;
    //set_process(opened);
    thread->start(callable_mp(this, &CanBus::threaded_operation), Thread::PRIORITY_NORMAL);

    return true;

}

void CanBus::threaded_operation() {
    
    while(opened){
        //call_deferred("receive");
        receive();
        usleep(1000 * 10);
    }
}


int CanBus::send(int canid, PackedByteArray buf, int len)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    //int len = sizeof(addr);
    struct can_frame frame;

    frame.can_id = canid;
    frame.len = len;
    memcpy(frame.data, buf.ptr(), len);

    strcpy(ifr.ifr_name, ifname);
    ioctl(canfd, SIOCGIFINDEX, &ifr);
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_family  = AF_CAN;

    int nbytes = sendto(canfd, &frame, sizeof(struct can_frame),
                    0, (struct sockaddr*)&addr, sizeof(addr));
    emit_signal("on_message_sent", nbytes);
    return nbytes;
}



int CanBus::receive()
{
    struct can_frame frame;
    //PackedByteArray pack = PackedByteArray();
    //pack.append('c');
        
    //memcpy(pack.ptrw(), frame.data, frame.len);
    //emit_signal("on_message_received", 0, pack, 1);
    //call_deferred("emit_signal","on_message_received", 0, pack, 1);

    //return 0;
    int nbytes = read(canfd, &frame, sizeof(struct can_frame));

    if(!opened){
        return 0;
    }
  
    if (nbytes < 0) {
        UtilityFunctions::printerr("can raw socket read");
        perror("can raw socket read");
    }
    else {
        PackedByteArray pack = PackedByteArray();
        for(int i = 0; i < frame.len; i++){
            pack.append(frame.data[i]);
        }
        //memcpy(pack.ptrw(), frame.data, frame.len);
        call_deferred("emit_signal", "on_message_received", (int)frame.can_id, pack, (int)frame.len);
    }
    return nbytes;
}
