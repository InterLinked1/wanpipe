cmd_/hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/wanec_iface.o := gcc -Wp,-MD,/hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/.wanec_iface.o.d -nostdinc -iwithprefix include -D__KERNEL__ -Iinclude  -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Os -fomit-frame-pointer -Wdeclaration-after-statement -pipe -msoft-float -m32 -fno-builtin-sprintf -fno-builtin-log2 -fno-builtin-puts  -mpreferred-stack-boundary=2 -fno-unit-at-a-time -march=pentium4 -mregparm=3 -Iinclude/asm-i386/mach-generic -Iinclude/asm-i386/mach-default -D__LINUX__ -DCONFIG_PRODUCT_WANPIPE_BASE  -DCONFIG_PRODUCT_WANPIPE_TDM_VOICE  -DCONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN -DCONFIG_PRODUCT_WANPIPE_AFT -DCONFIG_PRODUCT_WANPIPE_AFT_TE1 -DCONFIG_PRODUCT_WANPIPE_CODEC_SLINEAR_LAW -DCONFIG_WANPIPE_HWEC  -I. -I/root/2.3.4/wanpipe/patches/kdrivers/wanec -I/root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api -I/root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include -I/root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include  -I/root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/apilib -I/root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/apilib -I/root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/octrpc -I/root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api -I/root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/octdeviceapi/oct6100api -DENABLE_TONE_PLAY  -I//usr/include/wanpipe -I/usr/include/wanpipe/oct6100_api -I/usr/include/wanpipe/oct6100_api/oct6100api    -DMODULE -DKBUILD_BASENAME=wanec_iface -DKBUILD_MODNAME=wanec -c -o /hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/.tmp_wanec_iface.o /hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/wanec_iface.c

deps_/hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/wanec_iface.o := \
  /hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/wanec_iface.c \
  include/linux/wanpipe_includes.h \
    $(wildcard include/config/inet.h) \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/compiler.h \
  include/linux/compiler-gcc3.h \
  include/linux/compiler-gcc.h \
  include/linux/version.h \
  include/linux/module.h \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/kallsyms.h) \
  include/linux/sched.h \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/preempt.h) \
  include/asm/param.h \
  include/linux/capability.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/asm/posix_types.h \
  include/asm/types.h \
    $(wildcard include/config/highmem64g.h) \
    $(wildcard include/config/lbd.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/lockmeter.h) \
  include/linux/preempt.h \
  include/linux/linkage.h \
  include/asm/linkage.h \
    $(wildcard include/config/regparm.h) \
    $(wildcard include/config/x86/alignment/16.h) \
  include/linux/thread_info.h \
  include/linux/bitops.h \
  include/asm/bitops.h \
  include/asm/thread_info.h \
    $(wildcard include/config/debug/stack/usage.h) \
  include/asm/page.h \
    $(wildcard include/config/x86/use/3dnow.h) \
    $(wildcard include/config/x86/pae.h) \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/highmem4g.h) \
    $(wildcard include/config/x86/4g/vm/layout.h) \
    $(wildcard include/config/discontigmem.h) \
  include/asm/processor.h \
    $(wildcard include/config/x86/high/entry.h) \
    $(wildcard include/config/mk8.h) \
    $(wildcard include/config/mk7.h) \
  include/asm/vm86.h \
  include/asm/math_emu.h \
  include/asm/sigcontext.h \
  include/asm/segment.h \
  include/asm/cpufeature.h \
  include/asm/msr.h \
  include/asm/system.h \
    $(wildcard include/config/x86/cmpxchg.h) \
    $(wildcard include/config/x86/oostore.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
  /usr/lib/gcc/i386-redhat-linux/3.4.3/include/stdarg.h \
  include/asm/byteorder.h \
    $(wildcard include/config/x86/bswap.h) \
  include/linux/byteorder/little_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm/bug.h \
  include/asm-generic/bug.h \
  include/linux/cache.h \
  include/asm/cache.h \
    $(wildcard include/config/x86/l1/cache/shift.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
  include/linux/stringify.h \
  include/asm/spinlock.h \
    $(wildcard include/config/x86/ppro/fence.h) \
  include/asm/atomic.h \
    $(wildcard include/config/m386.h) \
  include/asm/rwlock.h \
  include/linux/timex.h \
    $(wildcard include/config/time/interpolation.h) \
  include/asm/timex.h \
    $(wildcard include/config/x86/elan.h) \
    $(wildcard include/config/x86/tsc.h) \
    $(wildcard include/config/x86/generic.h) \
  include/linux/time.h \
  include/linux/seqlock.h \
  include/asm/div64.h \
  include/linux/jiffies.h \
  include/linux/rbtree.h \
  include/linux/cpumask.h \
    $(wildcard include/config/hotplug/cpu.h) \
  include/linux/bitmap.h \
  include/linux/string.h \
  include/asm/string.h \
  include/asm/semaphore.h \
  include/linux/wait.h \
  include/linux/list.h \
  include/linux/prefetch.h \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/asm/rwsem.h \
  include/asm/ptrace.h \
    $(wildcard include/config/frame/pointer.h) \
  include/asm/mmu.h \
  include/linux/smp.h \
  include/asm/smp.h \
    $(wildcard include/config/x86/local/apic.h) \
    $(wildcard include/config/x86/io/apic.h) \
  include/asm/fixmap.h \
    $(wildcard include/config/x86/visws/apic.h) \
    $(wildcard include/config/x86/cyclone/timer.h) \
    $(wildcard include/config/acpi/boot.h) \
    $(wildcard include/config/pci/mmconfig.h) \
  include/asm/acpi.h \
    $(wildcard include/config/acpi/pci.h) \
    $(wildcard include/config/acpi/sleep.h) \
  include/asm/apicdef.h \
  include/asm/kmap_types.h \
  include/asm/mpspec.h \
  include/asm/mpspec_def.h \
  include/asm-i386/mach-generic/mach_mpspec.h \
  include/asm/io_apic.h \
    $(wildcard include/config/pci/msi.h) \
  include/asm/apic.h \
    $(wildcard include/config/x86/good/apic.h) \
  include/linux/pm.h \
    $(wildcard include/config/pm.h) \
  include/asm-i386/mach-generic/mach_apicdef.h \
  include/asm/genapic.h \
  include/linux/sem.h \
    $(wildcard include/config/sysvipc.h) \
  include/linux/ipc.h \
  include/asm/ipcbuf.h \
  include/asm/sembuf.h \
  include/linux/signal.h \
  include/asm/signal.h \
  include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/resource.h \
  include/asm/resource.h \
  include/linux/securebits.h \
  include/linux/fs_struct.h \
  include/linux/completion.h \
  include/linux/pid.h \
  include/linux/percpu.h \
  include/linux/slab.h \
    $(wildcard include/config/.h) \
  include/linux/gfp.h \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
  include/linux/numa.h \
  include/linux/topology.h \
  include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/kmalloc_sizes.h \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/large/allocs.h) \
  include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/param.h \
  include/linux/timer.h \
  include/linux/aio.h \
  include/linux/workqueue.h \
  include/linux/aio_abi.h \
  include/asm/current.h \
  include/linux/stat.h \
  include/asm/stat.h \
  include/linux/kmod.h \
    $(wildcard include/config/kmod.h) \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/elf.h \
  include/asm/elf.h \
  include/asm/user.h \
  include/asm/desc.h \
  include/asm/ldt.h \
  include/linux/utsname.h \
  include/linux/kobject.h \
  include/linux/sysfs.h \
    $(wildcard include/config/sysfs.h) \
  include/linux/kref.h \
  include/linux/moduleparam.h \
  include/asm/local.h \
  include/asm/module.h \
    $(wildcard include/config/m486.h) \
    $(wildcard include/config/m586.h) \
    $(wildcard include/config/m586tsc.h) \
    $(wildcard include/config/m586mmx.h) \
    $(wildcard include/config/m686.h) \
    $(wildcard include/config/mpentiumii.h) \
    $(wildcard include/config/mpentiumiii.h) \
    $(wildcard include/config/mpentiumm.h) \
    $(wildcard include/config/mpentium4.h) \
    $(wildcard include/config/mk6.h) \
    $(wildcard include/config/mcrusoe.h) \
    $(wildcard include/config/mwinchipc6.h) \
    $(wildcard include/config/mwinchip2.h) \
    $(wildcard include/config/mwinchip3d.h) \
    $(wildcard include/config/mcyrixiii.h) \
    $(wildcard include/config/mviac3/2.h) \
  include/linux/mm.h \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/pagealloc.h) \
    $(wildcard include/config/arch/gate/area.h) \
  include/linux/prio_tree.h \
  include/linux/fs.h \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/auditsyscall.h) \
  include/linux/limits.h \
  include/linux/kdev_t.h \
  include/linux/ioctl.h \
  include/asm/ioctl.h \
  include/linux/dcache.h \
  include/linux/rcupdate.h \
  include/linux/radix-tree.h \
  include/linux/audit.h \
    $(wildcard include/config/audit.h) \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  include/asm/fcntl.h \
  include/linux/err.h \
  include/asm/pgtable.h \
    $(wildcard include/config/highpte.h) \
  include/asm/pgtable-3level-defs.h \
  include/asm/pgtable-3level.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/swap.h) \
  include/linux/ctype.h \
  include/net/ip.h \
    $(wildcard include/config/ipv6.h) \
    $(wildcard include/config/ipv6/module.h) \
  include/linux/socket.h \
    $(wildcard include/config/compat.h) \
  include/asm/socket.h \
  include/asm/sockios.h \
  include/linux/sockios.h \
  include/linux/uio.h \
  include/linux/ip.h \
  include/net/sock.h \
  include/linux/netdevice.h \
    $(wildcard include/config/ax25.h) \
    $(wildcard include/config/ax25/module.h) \
    $(wildcard include/config/tr.h) \
    $(wildcard include/config/net/ipip.h) \
    $(wildcard include/config/netpoll.h) \
    $(wildcard include/config/net/poll/controller.h) \
    $(wildcard include/config/net/divert.h) \
    $(wildcard include/config/netpoll/trap.h) \
  include/linux/if.h \
  include/linux/hdlc/ioctl.h \
  include/linux/if_ether.h \
  include/linux/skbuff.h \
    $(wildcard include/config/netfilter.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/vlan/8021q.h) \
    $(wildcard include/config/vlan/8021q/module.h) \
    $(wildcard include/config/netfilter/debug.h) \
    $(wildcard include/config/hippi.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/net/cls/act.h) \
  include/linux/highmem.h \
  include/asm/cacheflush.h \
  include/asm/highmem.h \
  include/linux/interrupt.h \
  include/linux/hardirq.h \
  include/linux/smp_lock.h \
  include/asm/hardirq.h \
  include/linux/irq.h \
    $(wildcard include/config/arch/s390.h) \
  include/asm/irq.h \
    $(wildcard include/config/irqbalance.h) \
  include/asm-i386/mach-default/irq_vectors.h \
  include/asm-i386/mach-default/irq_vectors_limits.h \
  include/asm/hw_irq.h \
  include/linux/profile.h \
    $(wildcard include/config/profiling.h) \
  include/asm/sections.h \
  include/asm-generic/sections.h \
  include/linux/irq_cpustat.h \
  include/asm/tlbflush.h \
    $(wildcard include/config/x86/invlpg.h) \
    $(wildcard include/config/x86/switch/pagetables.h) \
  include/asm/atomic_kmap.h \
    $(wildcard include/config/debug/highmem.h) \
  include/linux/poll.h \
  include/asm/poll.h \
  include/asm/uaccess.h \
    $(wildcard include/config/x86/intel/usercopy.h) \
    $(wildcard include/config/x86/wp/works/ok.h) \
    $(wildcard include/config/x86/uaccess/indirect.h) \
  include/linux/net.h \
  include/net/checksum.h \
  include/asm/checksum.h \
  include/linux/in6.h \
  include/linux/if_packet.h \
  include/linux/device.h \
  include/linux/ioport.h \
  include/linux/notifier.h \
  include/linux/security.h \
    $(wildcard include/config/security/network.h) \
  include/linux/binfmts.h \
  include/linux/sysctl.h \
    $(wildcard include/config/tux/debug/blocking.h) \
  include/linux/shm.h \
  include/asm/shmparam.h \
  include/asm/shmbuf.h \
  include/linux/msg.h \
  include/asm/msgbuf.h \
  include/linux/netlink.h \
  include/linux/filter.h \
  include/net/dst.h \
    $(wildcard include/config/net/cls/route.h) \
    $(wildcard include/config/xfrm.h) \
  include/linux/rtnetlink.h \
  include/net/neighbour.h \
  include/linux/seq_file.h \
  include/linux/igmp.h \
  include/linux/in.h \
  include/net/flow.h \
  include/linux/inetdevice.h \
  include/linux/in_route.h \
  include/net/route.h \
  include/net/inetpeer.h \
  include/linux/route.h \
  include/net/arp.h \
  include/linux/if_arp.h \
  include/net/snmp.h \
  include/linux/snmp.h \
  include/linux/ipv6.h \
    $(wildcard include/config/ipv6/privacy.h) \
  include/linux/icmpv6.h \
  include/net/if_inet6.h \
  include/linux/tcp.h \
  include/linux/udp.h \
  include/net/protocol.h \
  include/linux/wireless.h \
  include/linux/inet.h \
  include/asm/io.h \
    $(wildcard include/config/x86/numaq.h) \
  include/asm-generic/iomap.h \
  include/linux/vmalloc.h \
  include/asm/delay.h \
  include/linux/pci.h \
    $(wildcard include/config/pci/names.h) \
    $(wildcard include/config/pci.h) \
    $(wildcard include/config/isa.h) \
    $(wildcard include/config/eisa.h) \
    $(wildcard include/config/pci/domains.h) \
  include/linux/mod_devicetable.h \
  include/linux/pci_ids.h \
  include/linux/dmapool.h \
  include/asm/scatterlist.h \
  include/asm/pci.h \
  include/asm-generic/pci-dma-compat.h \
  include/linux/dma-mapping.h \
  include/asm/dma-mapping.h \
  include/asm-generic/pci.h \
  include/linux/pkt_sched.h \
  include/linux/etherdevice.h \
  include/linux/random.h \
  include/net/inet_common.h \
  include/linux/wanpipe_defines.h \
  include/linux/wanpipe_version.h \
  include/linux/wanpipe_kernel.h \
  include/linux/wanpipe.h \
    $(wildcard include/config/product/wanpipe/tdm/voice.h) \
    $(wildcard include/config/product/wanpipe/generic.h) \
    $(wildcard include/config/product/wanpipe/annexg.h) \
  include/linux/wanpipe_debug.h \
  include/linux/wanpipe_common.h \
  include/linux/wanpipe_events.h \
  include/linux/wanpipe_cfg.h \
    $(wildcard include/config/x25.h) \
    $(wildcard include/config/fr.h) \
    $(wildcard include/config/ppp.h) \
    $(wildcard include/config/chdlc.h) \
    $(wildcard include/config/bsc.h) \
    $(wildcard include/config/hdlc.h) \
    $(wildcard include/config/mppp.h) \
    $(wildcard include/config/mprot.h) \
    $(wildcard include/config/bitstrm.h) \
    $(wildcard include/config/edukit.h) \
    $(wildcard include/config/ss7.h) \
    $(wildcard include/config/bscstrm.h) \
    $(wildcard include/config/mfr.h) \
    $(wildcard include/config/adsl.h) \
    $(wildcard include/config/sdlc.h) \
    $(wildcard include/config/atm.h) \
    $(wildcard include/config/pos.h) \
    $(wildcard include/config/aft.h) \
    $(wildcard include/config/debug.h) \
    $(wildcard include/config/adccp.h) \
    $(wildcard include/config/mlink/ppp.h) \
    $(wildcard include/config/generic.h) \
    $(wildcard include/config/aft/te3.h) \
    $(wildcard include/config/mpchdlc.h) \
    $(wildcard include/config/aft/te1/ss7.h) \
    $(wildcard include/config/lapb.h) \
    $(wildcard include/config/xdlc.h) \
    $(wildcard include/config/tty.h) \
    $(wildcard include/config/aft/te1.h) \
    $(wildcard include/config/xmtp2.h) \
    $(wildcard include/config/asyhdlc.h) \
    $(wildcard include/config/lip/atm.h) \
    $(wildcard include/config/aft/analog.h) \
    $(wildcard include/config/zap.h) \
    $(wildcard include/config/lapd.h) \
    $(wildcard include/config/eth.h) \
  include/linux/sdla_56k.h \
  include/linux/sdla_te1.h \
  include/linux/sdla_te3.h \
  include/linux/sdla_remora.h \
  include/linux/sdla_remora_proslic.h \
  include/linux/sdla_front_end.h \
  include/linux/wanrouter.h \
  include/linux/proc_fs.h \
    $(wildcard include/config/proc/devicetree.h) \
  include/linux/sdla_tdmv.h \
    $(wildcard include/config/product/wanpipe/tdm/voice/echomaster.h) \
    $(wildcard include/config/product/wanpipe/tdm/voice/dchan.h) \
  include/linux/sdlasfm.h \
  include/linux/sdladrv.h \
  include/linux/serial.h \
  include/linux/serialP.h \
    $(wildcard include/config/sbc8560.h) \
  include/linux/termios.h \
  include/asm/termios.h \
  include/asm/termbits.h \
  include/asm/ioctls.h \
  include/linux/circ_buf.h \
  include/linux/serial_reg.h \
  include/asm/serial.h \
    $(wildcard include/config/serial/detect/irq.h) \
    $(wildcard include/config/serial/many/ports.h) \
    $(wildcard include/config/hub6.h) \
    $(wildcard include/config/mca.h) \
  include/linux/tty.h \
    $(wildcard include/config/legacy/pty/count.h) \
  include/linux/major.h \
  include/linux/tty_driver.h \
  include/linux/cdev.h \
  include/linux/tty_ldisc.h \
  include/linux/tty_flip.h \
  include/linux/if_wanpipe.h \
    $(wildcard include/config/id.h) \
  include/linux/wanpipe_tdm_api.h \
  include/linux/wanpipe_codec_iface.h \
  include/linux/wanpipe_ec_kernel.h \
  /hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/wanec_iface.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_api.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/octdef.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/octosdependant.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/octtype.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_defines.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_errors.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_apiud.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_tlv_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_chip_stats_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_tsi_cnct_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_mixer_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_events_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_tone_detection_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_conf_bridge_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_playout_buf_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_adpcm_chan_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_phasing_tsst_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_channel_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_interrupts_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_remote_debug_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_debug_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_chip_open_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_api_inst.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_interrupts_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_tsi_cnct_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_events_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_tone_detection_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_mixer_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_conf_bridge_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_playout_buf_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_channel_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_remote_debug_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_debug_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_chip_open_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_chip_stats_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_adpcm_chan_pub.h \
  /root/2.3.4/wanpipe/patches/kdrivers/wanec/oct6100_api/include/oct6100api/oct6100_phasing_tsst_pub.h \

/hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/wanec_iface.o: $(deps_/hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/wanec_iface.o)

$(deps_/hda5/wanpipe/2.3.4/wanpipe/patches/kdrivers/wanec/wanec_iface.o):
