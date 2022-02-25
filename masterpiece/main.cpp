#include "SimpleSocket.hpp"
#include "BindingSocket.hpp"
#include "ConnectingSocket.hpp"
#include "ListeningSocket.hpp"
#include "SimpleServer.hpp"
#include "TestServer.hpp"


int main(void)
{
	// //htons(), htonl(), ntohs(), ntohl():
	// uint32_t some_long = 10;
	// uint16_t some_short = 20;

	// uint32_t network_byte_order;

	// // convert and send
	// network_byte_order = htonl(some_long);
	// //send(s, &network_byte_order, sizeof(uint32_t), 0);

	// if (some_short == ntohs(htons(some_short)))
	// 	std::cout << "True" << std::endl;

	//------------------------------------------------------------------------------------------
	// //select:
	// fd_set rfds;
	// struct timeval tv;
	// int retval;
	// /* Watch stdin (fd 0) to see when it has input. */

	// FD_ZERO(&rfds);
	// FD_SET(0, &rfds);

	// /* Wait up to five seconds. */

	// tv.tv_sec = 5;
	// tv.tv_usec = 0;

	// retval = select(1, &rfds, NULL, NULL, &tv);
	// /* Don't rely on the value of tv now! */
	// 	if (retval == -1)
	// 		perror("select()");
	// else if (retval)
	// 	printf("Data is available now.\n");
	// 	/* FD_ISSET(0, &rfds) will be true. */
	// else
	// 	printf("No data within five seconds.\n");

	// exit(EXIT_SUCCESS);

	//------------------------------------------------------------------------------------------
	// // poll:
	// #define MAX_EVENTS 10

	// struct epoll_event ev, events[MAX_EVENTS];
	// int listen_sock = 0, conn_sock, nfds, epollfd;

	// /* Code to set up listening socket, 'listen_sock',
	// 	(socket(), bind(), listen()) omitted */

	// epollfd = epoll_create1(0);
	// if (epollfd == -1) {
	// 	perror("epoll_create1");
	// 	exit(EXIT_FAILURE);
	// }

	// ev.events = EPOLLIN;
	// 		ev.data.fd = listen_sock;
	// if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
	// 	perror("epoll_ctl: listen_sock");
	// 	exit(EXIT_FAILURE);
	// }

	// for (;;)
	// {
	// 	nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
	// 	if (nfds == -1)
	// 	{
	// 		perror("epoll_wait");
	// 		exit(EXIT_FAILURE);
	// 	}

	// 	for (int n = 0; n < nfds; ++n)
	// 	{
	// 		if (events[n].data.fd == listen_sock)
	// 		{
	// 			conn_sock = accept(listen_sock, NULL, NULL);
	// 			if (conn_sock == -1)
	// 			{
	// 				perror("accept");
	// 				exit(EXIT_FAILURE);
	// 			}
	// 			ev.events = EPOLLIN | EPOLLET;
	// 			ev.data.fd = conn_sock;
	// 			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
	// 			{
	// 				perror("epoll_ctl: conn_sock");
	// 				exit(EXIT_FAILURE);
	// 			}
	// 		}
	// 	}
	// }

	//------------------------------------------------------------------------------------------
	// // kqueue:

	// struct	kevent event;	 /* Event we want to monitor */
	// struct	kevent tevent;	 /* Event triggered */
	// int kq, fd, ret;

	// if (argc != 2)
	// 	err(EXIT_FAILURE, "Usage: %s path\n", argv[0]);

	// fd = open(argv[1], O_RDONLY);
	// if (fd	== -1)
	// 	err(EXIT_FAILURE, "Failed to open '%s'", argv[1]);

	// /* Create kqueue. */
	// kq = kqueue();
	// if (kq	== -1)
	// 	err(EXIT_FAILURE, "kqueue() failed");

	// /* Initialize kevent structure. */
	// EV_SET(&event,	fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);

	// /* Attach event to the	kqueue.	*/
	// ret = kevent(kq, &event, 1, NULL, 0, NULL);
	// if (ret == -1)
	// 	err(EXIT_FAILURE, "kevent register");

	// if (event.flags & EV_ERROR)
	// 	errx(EXIT_FAILURE,	"Event error: %s", strerror(event.data));

	// for (;;)
	// {
	// 	/*	Sleep until something happens. */
	// 	ret = kevent(kq, NULL, 0, &tevent,	1, NULL);
	// 	if	(ret ==	-1)
	// 		err(EXIT_FAILURE, "kevent wait");
	// 	else if (ret > 0)
	// 		printf("Something was written in '%s'\n", argv[1]);
	// }

	//------------------------------------------------------------------------------------------
	//sockets --> see server/client

	TestServer t;




	return 0;
}
