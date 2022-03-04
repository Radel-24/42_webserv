Marcos:

```c++
FD_SET, FD_CLR, FD_ISSET, FD_ZERO

//Clears the bit for the file descriptor fd in the file descriptor set fdset.
void FD_CLR(int fd, fd_set *set);
//Returns a non-zero value if the bit for the file descriptor fd is set in the file descriptor set pointed to by fdset, and 0 otherwise.
int  FD_ISSET(int fd, fd_set *set);
//Sets the bit for the file descriptor fd in the file descriptor set fdset.
void FD_SET(int fd, fd_set *set);
//Initializes the file descriptor set fdset to have zero bits for all file descriptors.
void FD_ZERO(fd_set *set);

The behavior of these macros is undefined if the fd argument is less than 0 or greater than or equal to FD_SETSIZE, or if fd is not a valid file descriptor, or if any of the arguments are expressions with side effects.
````
--> see [[Functions#^b05748|select()]] for an example!

Functions:

```c++
htons():

    #include <arpa/inet.h>

    uint16_t htons(uint16_t hostshort);
	
	converts the unsigned short integer hostshort from host byte order to network byte order.


htonl():
				   
	#include <arpa/inet.h>

    uint32_t htonl(uint32_t hostlong);
				   
	converts the unsigned integer hostlong from host byte order to network byte order.


ntohs():
				   
	#include <arpa/inet.h>

    uint16_t ntohs(uint16_t netshort);
				   
	converts the unsigned short integer netshort from network byte order to host byte order.


ntohl():
				   
	#include <arpa/inet.h>
				   
	uint32_t ntohl(uint32_t netlong);
				   
	converts the unsigned integer netlong from network byte order to host byte order.
```
Refrence: https://www.gta.ufrj.br/ensino/eel878/sockets/htonsman.html

```c++
select():

	#include <sys/select.h>
	#include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>

	int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
				   
	WARNING: select() can monitor only file descriptors numbers that are less than FD_SETSIZE(1024)
			- an unreasonably low limit for many modern applications—and this limitation will not change.
			All modern applications should instead use [poll(2)] which do not suffer this limitation.
	
	allows a program to monitor multiple file descriptors,
	waiting until one or more of the file descriptors become "ready"
	for some class of I/O operation (e.g., input possible).  A file
	descriptor is considered ready if it is possible to perform a
	corresponding I/O operation (e.g., read(2), or a sufficiently
	small write(2)) without blocking.

	struct timeval
	{
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* microseconds */
	};

EXAMPLE:		   

	#include <stdio.h>
    #include <stdlib.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>

    int main(void)
    {
        fd_set rfds;
        struct timeval tv;
        int retval;
		/* Watch stdin (fd 0) to see when it has input. */

        FD_ZERO(&rfds);
        FD_SET(0, &rfds);

        /* Wait up to five seconds. */

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */
		if (retval == -1)
            perror("select()");
        else if (retval)
            printf("Data is available now.\n");
	        /* FD_ISSET(0, &rfds) will be true. */
        else
            printf("No data within five seconds.\n");

        exit(EXIT_SUCCESS);
    }

Arguments:

       The arguments of select() are as follows:

       readfds
              The file descriptors in this set are watched to see if
              they are ready for reading.  A file descriptor is ready
              for reading if a read operation will not block; in
              particular, a file descriptor is also ready on end-of-
              file.

              After select() has returned, _readfds_ will be cleared of
              all file descriptors except for those that are ready for
              reading.

	    writefds
              The file descriptors in this set are watched to see if
              they are ready for writing.  A file descriptor is ready
              for writing if a write operation will not block.  However,
              even if a file descriptor indicates as writable, a large
              write may still block.

              After select() has returned, _writefds_ will be cleared of
              all file descriptors except for those that are ready for
              writing.

       exceptfds
              The file descriptors in this set are watched for
              "exceptional conditions".  For examples of some
              exceptional conditions, see the discussion of POLLPRI in
              [poll(2)](https://man7.org/linux/man-pages/man2/poll.2.html).

              After select() has returned, exceptfds will be cleared of
              all file descriptors except for those for which an
              exceptional condition has occurred.

       nfds
			  This argument should be set to the highest-numbered file
              descriptor in any of the three sets, plus 1.  The
              indicated file descriptors in each set are checked, up to
              this limit (but see BUGS).

       timeout
              The timeout argument is a timeval structure (shown below)
              that specifies the interval that select() should block
              waiting for a file descriptor to become ready.  The call
              will block until either:

              • a file descriptor becomes ready;

              • the call is interrupted by a signal handler; or

              • the timeout expires.

              Note that the timeout interval will be rounded up to the
              system clock granularity, and kernel scheduling delays
              mean that the blocking interval may overrun by a small
              amount.

              If both fields of the timeval structure are zero, then
              select() returns immediately.  (This is useful for
              polling.)

              If timeout is specified as NULL, select() blocks
              indefinitely waiting for a file descriptor to become
              ready.
						
Return Value:

	   On success, select() return the number of file
       descriptors contained in the three returned descriptor sets (that
       is, the total number of bits that are set in readfds, writefds,
       exceptfds).  The return value may be zero if the timeout expired
       before any file descriptors became ready.

       On error, -1 is returned, and [errno]
       the file descriptor sets are unmodified, and timeout becomes
       undefined.

```

^b05748

```c++
poll():
				   
	#include <poll.h>

    int poll(struct pollfd *fds, nfds_t nfds, int timeout);
				   
	performs a similar task to select(2): it waits for one of
	a set of file descriptors to become ready to perform I/O.  The
	Linux-specific epoll(7) API performs a similar task, but offers
	features beyond those found in poll().

	struct pollfd
	{
	       int   fd;         /* file descriptor */
	       short events;     /* requested events */
	       short revents;    /* returned events */
	};


epoll (epoll_create, epoll_ctl, epoll_wait):

	#include <sys/epoll.h>

	The epoll API performs a similar task to poll(2): monitoring
	multiple file descriptors to see if I/O is possible on any of
	them.  The epoll API can be used either as an edge-triggered or a
	level-triggered interface and scales well to large numbers of
	watched file descriptors.

	The central concept of the epoll API is the epoll instance, an
	in-kernel data structure which, from a user-space perspective,
	can be considered as a container for two lists:

       • The interest list (sometimes also called the epoll set): the
         set of file descriptors that the process has registered an
         interest in monitoring.

       • The ready list: the set of file descriptors that are "ready"
         for I/O.  The ready list is a subset of (or, more precisely, a
         set of references to) the file descriptors in the interest
         list.  The ready list is dynamically populated by the kernel as
         a result of I/O activity on those file descriptors.

       The following system calls are provided to create and manage an
       epoll instance:

       • epoll_create(2) creates a new epoll instance and returns a file
         descriptor referring to that instance.  (The more recent
         epoll_create1(2) extends the functionality of epoll_create(2).)

       • Interest in particular file descriptors is then registered via
         epoll_ctl(2), which adds items to the interest list of the
         epoll instance.

       • epoll_wait(2) waits for I/O events, blocking the calling thread
         if no events are currently available.  (This system call can be
         thought of as fetching items from the ready list of the epoll
         instance.)
				   
	EXAMPLE CODE:
				   
		#define MAX_EVENTS 10
				   
        struct epoll_event ev, events[MAX_EVENTS];
        int listen_sock, conn_sock, nfds, epollfd;

        /* Code to set up listening socket, 'listen_sock',
           (socket(), bind(), listen()) omitted */

        epollfd = epoll_create1(0);
        if (epollfd == -1) {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }

        ev.events = EPOLLIN;
			   ev.data.fd = listen_sock;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
            perror("epoll_ctl: listen_sock");
            exit(EXIT_FAILURE);
        }

        for (;;)
		{
            nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
            if (nfds == -1)
			{
                perror("epoll_wait");
                exit(EXIT_FAILURE);
            }

		    for (n = 0; n < nfds; ++n)
			{
                   if (events[n].data.fd == listen_sock)
					{
                       conn_sock = accept(listen_sock,(struct sockaddr *) &addr, &addrlen);
						if (conn_sock == -1)
						{
                           perror("accept");
                           exit(EXIT_FAILURE);
	                    }
                       setnonblocking(conn_sock);
                       ev.events = EPOLLIN | EPOLLET;
                       ev.data.fd = conn_sock;
                       if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
						{
                           perror("epoll_ctl: conn_sock");
                           exit(EXIT_FAILURE);
	                    }
				    } 
					else 
                       do_use_fd(events[n].data.fd);
            }
           
Return Value:
       On success, poll() returns a nonnegative value which is the
       number of elements in the pollfds whose revents fields have been
       set to a nonzero value (indicating an event or an error).  A
       return value of zero indicates that the system call timed out
       before any file descriptors became read.

       On error, -1 is returned, and [errno]

```

```c++
kqueue (kqueue, kevent):

    #include <sys/types.h>
	#include <sys/event.h>
	#include <sys/time.h>

	int kqueue(void);
	int keven(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);

	The kqueue() system call provides a generic method of notifying the user when an event happens or a
	condition holds, based on the results of small pieces of kernel code termed filters.
	A kevent is identified by the (ident, filter) pair; there may only be one unique kevent per kqueue.

	struct kevent
	{
		uintptr_t   ident;	/* identifier for this event */
		short	    filter;	/* filter for event */
		u_short	    flags;	/* action flags for kqueue */
		u_int	    fflags;	/* filter flag value */
		int64_t	    data;	/* filter data value */
		void	   *udata;	/* opaque user data identifier */
	};

Example:

     #include <sys/event.h>
     #include <err.h>
     #include <fcntl.h>
     #include <stdio.h>
     #include <stdlib.h>
     #include <string.h>

     int
     main(int argc, char **argv)
     {
		 struct	kevent event;	 /* Event we want to monitor */
		 struct	kevent tevent;	 /* Event triggered */
		 int kq, fd, ret;

		 if (argc != 2)
		     err(EXIT_FAILURE, "Usage: %s path\n", argv[0]);
		 fd = open(argv[1], O_RDONLY);
		 if (fd	== -1)
		     err(EXIT_FAILURE, "Failed to open '%s'", argv[1]);

		 /* Create kqueue. */
		 kq = kqueue();
		 if (kq	== -1)
		     err(EXIT_FAILURE, "kqueue() failed");

		 /* Initialize kevent structure. */
		 EV_SET(&event,	fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);
		 /* Attach event to the	kqueue.	*/
		 ret = kevent(kq, &event, 1, NULL, 0, NULL);
		 if (ret == -1)
		     err(EXIT_FAILURE, "kevent register");
		 if (event.flags & EV_ERROR)
		     errx(EXIT_FAILURE,	"Event error: %s", strerror(event.data));

		 for (;;)
		{
		    /*	Sleep until something happens. */
		    ret = kevent(kq, NULL, 0, &tevent,	1, NULL);
		    if	(ret ==	-1)
				err(EXIT_FAILURE, "kevent wait");
			else if (ret > 0)
				printf("Something was written in '%s'\n", argv[1]);
		 }
	}


Return Value:
     The kqueue() system call creates a	new kernel event queue and returns a
     file descriptor.  If there	was an error creating the kernel event queue,
     a value of	-1 is returned and errno set.

     The kevent() system call returns the number of events placed in the
     eventlist,	up to the value	given by nevents.  If an error occurs while
     processing	an element of the changelist and there is enough room in the
     eventlist,	then the event will be placed in the eventlist with EV_ERROR
     set in flags and the system error in data.	 Otherwise, -1 will be re-
     turned, and errno will be set to indicate the error condition.  If	the
     time limit	expires, then kevent() returns 0.

```

```c++
socket():

	#include <sys/types.h>
    #include <sys/socket.h>

	int socket(int domain, int type, int protocol);

	creates an endpoint for communication and returns a file
	descriptor that refers to that endpoint.  The file descriptor
	returned by a successful call will be the lowest-numbered file
	descriptor not currently open for the process.

	The domain argument specifies a communication domain; this
	selects the protocol family which will be used for communication.
	These families are defined in <sys/socket.h>.

Return Value:
     A -1 is returned if an error occurs, otherwise the	return value is	a de-
     scriptor referencing the socket.



accept():

	#include <sys/types.h>
    #include <sys/socket.h>

    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

	The accept() system call is used with connection-based socket
	types (SOCK_STREAM, SOCK_SEQPACKET).  It extracts the first
	connection request on the queue of pending connections for the
	listening socket, sockfd, creates a new connected socket, and
	returns a new file descriptor referring to that socket.  The
	newly created socket is not in the listening state.  The original
	socket sockfd is unaffected by this call.

Return Value:
	These calls return	-1 on error.  If they succeed, they return a non-nega-
    tive integer that is a descriptor for the accepted	socket.


listen():

	#include <sys/types.h>
    #include <sys/socket.h>

    int listen(int sockfd, int backlog);

	listen() marks the socket referred to by sockfd as a passive
	socket, that is, as a socket that will be used to accept incoming
	connection requests using accept(2).
	The sockfd argument is a file descriptor that refers to a socket
	of type SOCK_STREAM or SOCK_SEQPACKET.

Return Value:
     The listen() function returns the value 0 if successful; otherwise	the
     value -1 is returned and the global variable errno	is set to indicate the
     error.

send():

	#include <sys/types.h>
    #include <sys/socket.h>

    ssize_t send(int sockfd, const void *buf, size_t len, int flags);

	The system calls send(), sendto(), and sendmsg() are used to
	transmit a message to another socket.

	The send() call may be used only when the socket is in a
	connected state (so that the intended recipient is known).  The
	only difference between send() and write(2) is the presence of
	flags.  With a zero flags argument, send() is equivalent to write(2).

Return Value:
     The send(), sendto() and sendmsg()	calls return the number	of octets
     sent.  The	sendmmsg() call	returns	the number of messages sent.  If an
     error occurred a value of -1 is returned.

recv():

	#include <sys/types.h>
    #include <sys/socket.h>

    ssize_t recv(int sockfd, void *buf, size_t len, int flags);

	The recv(), recvfrom(), and recvmsg() calls are used to receive
	messages from a socket.  They may be used to receive data on both
	connectionless and connection-oriented sockets.  This page first
	describes common features of all three system calls, and then
	describes the differences between the calls.

	The only difference between recv() and read(2) is the presence of
	flags.  With a zero flags argument, recv() is generally equivalent to read(2)

Return Value:
	 recv() return the number of bytes received. A value of -1	is returned if an error	occurred.

bind():

	#include <sys/types.h>
    #include <sys/socket.h>

    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

	When a socket is created with socket(2), it exists in a name
	space (address family) but has no address assigned to it.  bind()
	assigns the address specified by addr to the socket referred to
	by the file descriptor sockfd.  addrlen specifies the size, in
	bytes, of the address structure pointed to by addr.
	Traditionally, this operation is called "assigning a name to a socket".

	struct sockaddr
	{
	    sa_family_t sa_family;
	    char        sa_data[14];
    }

Return Value:
     The bind()	function returns the value 0 if	successful; otherwise the
     value -1 is returned and the global variable errno	is set to indicate the
     error.

connect():

	#include <sys/types.h>
    #include <sys/socket.h>

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

	The connect() system call connects the socket referred to by the
	file descriptor sockfd to the address specified by addr.  The
	addrlen argument specifies the size of addr.  The format of the
	address in addr is determined by the address space of the socket
	sockfd; see socket(2) for further details.

Return Value:
     The connect() function returns the	value 0	if successful; otherwise the
     value -1 is returned and the global variable errno	is set to indicate the
     error.

inet_addr():

	#include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

    in_addr_t inet_addr(const char *cp);

	The inet_addr() function shall convert the string pointed to by
	cp, in the standard IPv4 dotted decimal notation, to an integer
	value suitable for use as an Internet address.


setsockopt():

	#include <sys/types.h>
    #include <sys/socket.h>

    int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

	The setsockopt() function shall set the option specified by the
	option_name argument, at the protocol level specified by the
	level argument, to the value pointed to by the option_value
	argument for the socket associated with the file descriptor
	specified by the socket argument.

Return Value:
     Upon successful completion, the value 0 is	returned; otherwise the
     value -1 is returned and the global variable errno	is set to indicate the
     error.

getsockname():

	#include <sys/types.h>
    #include <sys/socket.h>	
    
    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);

	getsockname() returns the current address to which the socket
	sockfd is bound, in the buffer pointed to by addr.  The addrlen
	argument should be initialized to indicate the amount of space
	(in bytes) pointed to by addr.  On return it contains the actual
	size of the socket address.

	The returned address is truncated if the buffer provided is too
	small; in this case, addrlen will return a value greater than was
	supplied to the call.

Return Value:
     The getsockname() function	returns	the value 0 if successful; otherwise
     the value -1 is returned and the global variable errno is set to indicate
     the error.

```
 ![[Pasted image 20220224140918.png]]
Reference: https://www.geeksforgeeks.org/socket-programming-cc/
https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
Videoseries for cpp web server: https://www.youtube.com/watch?v=YwHErWJIh6Y

```c++
fcntl():

    #include <unistd.h>
    #include <fcntl.h>

    //int fcntl(int fd, int cmd, ... /* arg */ );
	--> int fcntl(fd, F_SETFL, O_NONBLOCK);

O_NONBLOCK:
	Non-blocking I/O; if no data is available to a read(2) system call, or if a write(2) operation would block, the	read or write call	returns	-1 with	the error EAGAIN.
F_SETFL:
	Set descriptor status flags to arg.

	fcntl - manipulate file descriptor

	fcntl() performs one of the operations described below on the open file descriptor fd.  The operation is determined by cmd.

    fcntl()  can take an optional third argument.  Whether or not this argument is required is determined by cmd.  The required argument type is indicated in parentheses after
	each cmd name (in most cases, the required type is int, and we identify the argument using the name arg), or void is specified if the argument is not required.

    Certain of the operations below are supported only since a particular Linux kernel version.  The preferred method of checking whether the host kernel supports a particular
	operation is  to  invoke  fcntl()
    with the desired cmd value and then test whether the call failed with EINVAL, indicating that the kernel does not recognize this value.

```

^de79bf

