#ifndef COMMAND_H
#define COMMAND_H

#define BUFF_SIZE           23

struct CNG_DATA {
    uint16_t      header;
    uint8_t       action;
    uint8_t       isAdmin;
    uint16_t      id;
    uint32_t      timestamp;
    uint32_t      expire;
    uint32_t      cardNo;
    uint8_t       main_version;
    uint8_t       min_version;
    uint8_t       crc;
    uint16_t      tailer;
};

union RemoteCommand {
    unsigned char buff[BUFF_SIZE];
    struct CNG_DATA data;
};

class Command {
	public :
        RemoteCommand command;

        Command ();
        inline void clear_command ();
        inline bool check_command ();
        inline bool is_ready ();
	private :
		int pos = 0;
		
};

/********************* implementations ****************/
Command::Command () {
    memset (command.buff, 0, BUFF_SIZE);
}

inline void Command::clear_command () {
    memset (command.buff, 0, BUFF_SIZE);
    pos = 0;
}

inline bool Command::check_command () {
    if (command.data.header != 0xCAFE) {
        return false;
    }

    if (command.data.tailer != 0xBABE) {
        return false;
    }

    int sum = 0;
    for (int i = 2; i < 20; i ++) {
        sum += command.buff [i];
    }
    sum &= 0xff;
    if (sum != command.data.crc) {
        return false;
    }

    return true;
}

inline bool Command::is_ready () {
    int ch = Serial.read ();
    if (ch >= 0) {
        command.buff [pos ++] = ch;
        if (pos >= BUFF_SIZE) {
            return check_command ();
        }
    }
    return false;
}
#endif
