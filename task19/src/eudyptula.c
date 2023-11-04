// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>

#include <linux/textsearch.h>

#if 1
#define MY_DEBUG(str, ...)                                                     \
	pr_alert("%s:%s():%d: " str "\n", THIS_MODULE->name, __func__,         \
		 __LINE__, ##__VA_ARGS__)
#else
#define MY_DEBUG(str, ...)
#endif

static const char *id_str = "eudyptula";
static struct ts_config *ts_conf;

static void pr_alert_buf_ascii(const char *pre, const char *start,
			       const char *end)
{
	const char *c;

	pr_alert("%s", pre);
	for (c = start; c < end; c++) {
		if (*c >= 32 && *c <= 126)
			pr_cont("%c", *c);
		else
			pr_cont(".");
	}
	pr_cont("\n");
}

static unsigned int eudyptula_nf_hook_op(void *priv, struct sk_buff *skb,
					 const struct nf_hook_state *state)
{
	struct iphdr *ip_header;
	struct udphdr *udp_header;
	struct tcphdr *tcp_header;
	struct icmphdr *icmp_header;
	unsigned int src_ip, dest_ip, src_port, dest_port;
	unsigned int ts_offset;

	ip_header = (struct iphdr *)skb_network_header(skb);
	src_ip = ntohl((unsigned int)ip_header->saddr);
	dest_ip = ntohl((unsigned int)ip_header->daddr);
	src_port = 0;
	dest_port = 0;
	if (ip_header->protocol == 17) {
		udp_header = (struct udphdr *)(skb_transport_header(skb) + 20);
		src_port = (unsigned int)ntohs(udp_header->source);
		dest_port = (unsigned int)ntohs(udp_header->dest);
	} else if (ip_header->protocol == 6) {
		tcp_header = (struct tcphdr *)(skb_transport_header(skb) + 20);
		src_port = (unsigned int)ntohs(tcp_header->source);
		dest_port = (unsigned int)ntohs(tcp_header->dest);
	}

	pr_alert(
		"IPv4 packet src: %u.%u.%u.%u:%u  dest: %u.%u.%u.%u:%u  protocol: %u",
		(src_ip >> 24) & 0xff, (src_ip >> 16) & 0xff,
		(src_ip >> 8) & 0xff, (src_ip >> 0) & 0xff, src_port,
		(dest_ip >> 24) & 0xff, (dest_ip >> 16) & 0xff,
		(dest_ip >> 8) & 0xff, (dest_ip >> 0) & 0xff, dest_port,
		ip_header->protocol);
	switch (ip_header->protocol) {
	case IPPROTO_ICMP:
		pr_cont(" (ICMP: ");
		icmp_header = (struct icmphdr *)((char *)ip_header +
						 sizeof(struct iphdr));
		switch (icmp_header->type) {
		case ICMP_ECHOREPLY:
			pr_cont("Echo Reply");
			break;
		case ICMP_DEST_UNREACH:
			pr_cont("Destination Unreachable");
			break;
		case ICMP_SOURCE_QUENCH:
			pr_cont("Source Quench");
			break;
		case ICMP_REDIRECT:
			pr_cont("Redirect (change route)");
			break;
		case ICMP_ECHO:
			pr_cont("Echo Request");
			break;
		case ICMP_TIME_EXCEEDED:
			pr_cont("Time Exceeded");
			break;
		case ICMP_PARAMETERPROB:
			pr_cont("Parameter Problem");
			break;
		case ICMP_TIMESTAMP:
			pr_cont("Timestamp Request");
			break;
		case ICMP_TIMESTAMPREPLY:
			pr_cont("Timestamp Reply");
			break;
		case ICMP_INFO_REQUEST:
			pr_cont("Information Request");
			break;
		case ICMP_INFO_REPLY:
			pr_cont("Information Reply");
			break;
		case ICMP_ADDRESS:
			pr_cont("Address Mask Request");
			break;
		case ICMP_ADDRESSREPLY:
			pr_cont("Address Mask Reply");
			break;
		default:
			pr_cont("???");
		}
		pr_cont(")");
		break;
	case IPPROTO_TCP:
		pr_cont(" (TCP)");
		break;
	case IPPROTO_UDP:
		pr_cont(" (UDP)");
		break;
	default:
		pr_cont(" (???)");
	}
	ts_offset = skb_find_text(skb, 0, skb->len, ts_conf);
	if (ts_offset != UINT_MAX)
		pr_cont(", id string found at offset 0x%x", ts_offset);
	pr_cont("\n");

	return NF_ACCEPT;
}

static struct nf_hook_ops eudyptula_nf_hook_ops = {
	.hook = eudyptula_nf_hook_op,
	.hooknum = NF_INET_PRE_ROUTING,
	.pf = NFPROTO_IPV4,
	.priority = NF_IP_PRI_FIRST,
};

static int __init eudyptula_init(void)
{
	int err;

	MY_DEBUG("Module loading...");

	err = nf_register_net_hook(&init_net, &eudyptula_nf_hook_ops);
	if (err) {
		pr_err("nf_register_net_hook() failed with err: %d", err);
		goto out1;
	}

	ts_conf = textsearch_prepare("kmp", // algo
				     id_str, strlen(id_str), // search string
				     GFP_KERNEL, // alloc flags
				     TS_AUTOLOAD); // search flags
	if (IS_ERR(ts_conf)) {
		err = PTR_ERR(ts_conf);
		pr_err("textsearch_prepare() failed with err: %d", err);
		goto out2;
	}

	MY_DEBUG("Module loaded");

	return 0;

out2:
	nf_unregister_net_hook(&init_net, &eudyptula_nf_hook_ops);
out1:
	return err;
}

static void __exit eudyptula_exit(void)
{
	nf_unregister_net_hook(&init_net, &eudyptula_nf_hook_ops);
	textsearch_destroy(ts_conf);
	MY_DEBUG("Module unloaded");
}

module_init(eudyptula_init);
module_exit(eudyptula_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Scott J. Crouch");
MODULE_DESCRIPTION("Example use of netfilter for IPv4 packet inspection");
