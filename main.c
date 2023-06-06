#include "main.h" 

struct termios saved_attributes;

void reset_input_mode (void)
{
    tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode (void)
{
    struct termios tattr;
    tcgetattr (STDIN_FILENO, &saved_attributes);
    tcgetattr (STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON|ECHO);
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

void setCursor(int pos, char* buf){
    int x = 0, y = 0; 
    char str[16];
    for(int i = 0; i < pos; i++){
        if(buf[i] == '\n') {
            y++;
            x = 0;
        }
        else {
            x++;
        }
    }
    write(STDOUT_FILENO, str, sprintf(str, "\e[%d;%dH", y+1, x+1));
}

void stopit(void){
    reset_input_mode();
    putchar('\n');
}

int main(int argc, char* argv[]){
    int fd = 0, x = 0, y = 0, sz, maxsz;
    char buf[4095], inbuf[3];

    atexit(stopit);
    set_input_mode();
    
    if(argc > 1){
        fd = open(argv[1], O_RDWR | O_CREAT, S_IRWXU);
        if(fd != -1){
            maxsz = sz = read(fd, buf, sizeof buf);
            write(STDOUT_FILENO, "\e[2J\e[H", 8);
            write(STDOUT_FILENO, buf, sz);  
        }
        else {
            write(STDERR_FILENO, "can't open\n", 12);
            exit(0);
        }
    }
    else {
        write(STDERR_FILENO, "few argc\n", 10);
        exit(0);
    }

    while(fd > 0){
        read(STDIN_FILENO, inbuf, 3);

        switch (inbuf[0])
        {
            case '\177':
                if(sz > 0)
                    buf[--sz] = ' ';
                break;
            case 27:
                if(inbuf[1] == 91){
                    switch (inbuf[2])
                    {
                        case 68: //left
                            if(sz > 0) sz--;
                            break;
                        case 67: //right
                            if(sz < maxsz) sz++;
                            break;
                        case 65: //up
                            while(buf[sz--] != '\n' && sz > 0);
                            break;
                        case 66: //down
                            if(sz < maxsz)
                                while(buf[sz++] != '\n' && sz < maxsz);
                            break;        
                    }
                }
                break;
            case ':':
                lseek(fd, 0, SEEK_SET);
                if(write(fd, buf, maxsz) == -1)
                    write(STDERR_FILENO, "write err\n", 11);
                break;
            case '~':
                if(close(fd) == -1) 
                    write(STDERR_FILENO, "close err\n", 11);
                exit(0);
                break;
            default:
                buf[sz] = inbuf[0];
                sz++;
                if(maxsz < sz) 
                    maxsz = sz;
                break;
        }

        write(STDOUT_FILENO, "\e[2J\e[H", 8);
        write(STDOUT_FILENO, buf, maxsz);
        setCursor(sz, buf);
    }    
    return 0;
}

