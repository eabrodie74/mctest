# mctest.c - IPv4 Multicast Testing Tool

Multicast Testing Tool is compatible with Windows, Linux and Mac OS operating systems.

Out of frustration with the absence of utilities written for Windows and Mac OS that test
multicast connectivity across a PIM routed network, I decided to dive into C socket
programming and learn as much as possible within one month, during which I needed a way
of testing bidirectional multicast flow between Amazon's AWS cloud and bare metal. The
only Windows-based multicast testing tools that I could find online were proprietary
binaries created for specific video conferencing or telephony applications with hard-
coded multicast groups. This made it near impossible to arbitrarily test multicast flow
in any direction I wanted, with any multicast group that I wanted that was already
configured for PIM routing between multiple switches across bare-metal/cloud boundaries
and evne across multiple AWS regions.

mctest can be compiled in all major operating systems, and allows for transmitting or
receiving multicast packets in any arbitrary multicast group, from any available
specified network interface over any specified UDP port.

This is a work in progress. While I greduated with a BS in computer science in 1997, I
haven't programmed since, and this is my first deep-dive into simple coding that I
intend to refine going forward. The code is not complex and is contained within a
very small monolithic file.

Any constructive feedback is more than welcomed!
