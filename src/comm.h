/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  COMM.H                                       Based on CircleMUD *
*  Usage: Header: Protos of Public Communication Functions                *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#ifndef __COMM_H__             /* prevent duplicate inclusions */
#define __COMM_H__

#define NUM_RESERVED_DESCS      8

/* comm.c */
void    send_to_all(char *messg);
void    send_to_char(char *messg, struct char_data *ch);
void    send_to_room(char *messg, int room);
void    send_to_zone(char *messg, int zone);
void    send_to_outdoor(char *messg);
void    perform_to_all(char *messg, struct char_data *ch);
void    close_socket(struct descriptor_data *d);

void    perform_act(char *orig, struct char_data *ch, struct obj_data *obj,
		    void *vict_obj, struct char_data *to);

void    act(char *str, int hide_invisible, struct char_data *ch,
struct  obj_data *obj, void *vict_obj, int type);

#define TO_ROOM         1
#define TO_VICT         2
#define TO_NOTVICT      3
#define TO_CHAR         4
#define TO_SLEEP        128     /* to char, even if sleeping */

int     write_to_descriptor(int desc, char *txt);
void    write_to_q(char *txt, struct txt_q *queue, int aliased);
void    write_to_output(const char *txt, struct descriptor_data *d);
void    page_string(struct descriptor_data *d, char *str, int keep_internal);

void 	send_to_q(const  char *txt, struct descriptor_data *d);
/* #define SEND_TO_Q(messg, desc)  write_to_output((messg), desc)
 * infobar1 (new SEND_TO_Q macro for send_to_q function)
 * notes: this won't affect how you use SEND_TO_Q, it just allows
 *        windowing output and the input line.
 */
#define SEND_TO_Q(messg, desc)  send_to_q((messg), desc)

#define USING_SMALL(d)  ((d)->output == (d)->small_outbuf)
#define USING_LARGE(d)  (!USING_SMALL(d))

typedef RETSIGTYPE sigfunc(int);

#endif /* _COMM_H */
