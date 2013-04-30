/*
 * Copyright IBM Corp. 2008, 2009
 *
 * Author(s): Heiko Carstens <heiko.carstens@de.ibm.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/ipl.h>
#include <asm/sclp.h>
#include <asm/setup.h>

#define ADDR2G (1ULL << 31)

static void find_memory_chunks(struct mem_chunk chunk[], unsigned long maxsize)
{
	unsigned long long memsize, rnmax, rzm;
	unsigned long addr = 0, size;
	int i = 0, type;

	rzm = sclp_get_rzm();
	rnmax = sclp_get_rnmax();
	memsize = rzm * rnmax;
	if (!rzm)
		rzm = 1ULL << 17;
	if (sizeof(long) == 4) {
		rzm = min(ADDR2G, rzm);
		memsize = memsize ? min(ADDR2G, memsize) : ADDR2G;
	}
	if (maxsize)
		memsize = memsize ? min((unsigned long)memsize, maxsize) : maxsize;
	do {
		size = 0;
		type = tprot(addr);
		do {
			size += rzm;
			if (memsize && addr + size >= memsize)
				break;
		} while (type == tprot(addr + size));
		if (type == CHUNK_READ_WRITE || type == CHUNK_READ_ONLY) {
			if (memsize && (addr + size > memsize))
				size = memsize - addr;
			chunk[i].addr = addr;
			chunk[i].size = size;
			chunk[i].type = type;
			i++;
		}
		addr += size;
	} while (addr < memsize && i < MEMORY_CHUNKS);
}

/**
 * detect_memory_layout - fill mem_chunk array with memory layout data
 * @chunk: mem_chunk array to be filled
 * @maxsize: maximum address where memory detection should stop
 *
 * Fills the passed in memory chunk array with the memory layout of the
 * machine. The array must have a size of at least MEMORY_CHUNKS and will
 * be fully initialized afterwards.
 * If the maxsize paramater has a value > 0 memory detection will stop at
 * that address. It is guaranteed that all chunks have an ending address
 * that is smaller than maxsize.
 * If maxsize is 0 all memory will be detected.
 */
void detect_memory_layout(struct mem_chunk chunk[], unsigned long maxsize)
{
	unsigned long flags, flags_dat, cr0;

	memset(chunk, 0, MEMORY_CHUNKS * sizeof(struct mem_chunk));
	/*
	 * Disable IRQs, DAT and low address protection so tprot does the
	 * right thing and we don't get scheduled away with low address
	 * protection disabled.
	 */
	local_irq_save(flags);
	flags_dat = __arch_local_irq_stnsm(0xfb);
	/*
	 * In case DAT was enabled, make sure chunk doesn't reside in vmalloc
	 * space. We have disabled DAT and any access to vmalloc area will
	 * cause an exception.
	 * If DAT was disabled we are called from early ipl code.
	 */
	if (test_bit(5, &flags_dat)) {
		if (WARN_ON_ONCE(is_vmalloc_or_module_addr(chunk)))
			goto out;
	}
	__ctl_store(cr0, 0, 0);
	__ctl_clear_bit(0, 28);
	find_memory_chunks(chunk, maxsize);
	__ctl_load(cr0, 0, 0);
out:
	__arch_local_irq_ssm(flags_dat);
	local_irq_restore(flags);
}
EXPORT_SYMBOL(detect_memory_layout);

/*
 * Move memory chunks array from index "from" to index "to"
 */
static void mem_chunk_move(struct mem_chunk chunk[], int to, int from)
{
	int cnt = MEMORY_CHUNKS - to;

	memmove(&chunk[to], &chunk[from], cnt * sizeof(struct mem_chunk));
}

/*
 * Initialize memory chunk
 */
static void mem_chunk_init(struct mem_chunk *chunk, unsigned long addr,
			   unsigned long size, int type)
{
	chunk->type = type;
	chunk->addr = addr;
	chunk->size = size;
}

/*
 * Create memory hole with given address, size, and type
 */
void create_mem_hole(struct mem_chunk chunk[], unsigned long addr,
		     unsigned long size, int type)
{
	unsigned long lh_start, lh_end, lh_size, ch_start, ch_end, ch_size;
	int i, ch_type;

	for (i = 0; i < MEMORY_CHUNKS; i++) {
		if (chunk[i].size == 0)
			continue;

		/* Define chunk properties */
		ch_start = chunk[i].addr;
		ch_size = chunk[i].size;
		ch_end = ch_start + ch_size - 1;
		ch_type = chunk[i].type;

		/* Is memory chunk hit by memory hole? */
		if (addr + size <= ch_start)
			continue; /* No: memory hole in front of chunk */
		if (addr > ch_end)
			continue; /* No: memory hole after chunk */

		/* Yes: Define local hole properties */
		lh_start = max(addr, chunk[i].addr);
		lh_end = min(addr + size - 1, ch_end);
		lh_size = lh_end - lh_start + 1;

		if (lh_start == ch_start && lh_end == ch_end) {
			/* Hole covers complete memory chunk */
			mem_chunk_init(&chunk[i], lh_start, lh_size, type);
		} else if (lh_end == ch_end) {
			/* Hole starts in memory chunk and convers chunk end */
			mem_chunk_move(chunk, i + 1, i);
			mem_chunk_init(&chunk[i], ch_start, ch_size - lh_size,
				       ch_type);
			mem_chunk_init(&chunk[i + 1], lh_start, lh_size, type);
			i += 1;
		} else if (lh_start == ch_start) {
			/* Hole ends in memory chunk */
			mem_chunk_move(chunk, i + 1, i);
			mem_chunk_init(&chunk[i], lh_start, lh_size, type);
			mem_chunk_init(&chunk[i + 1], lh_end + 1,
				       ch_size - lh_size, ch_type);
			break;
		} else {
			/* Hole splits memory chunk */
			mem_chunk_move(chunk, i + 2, i);
			mem_chunk_init(&chunk[i], ch_start,
				       lh_start - ch_start, ch_type);
			mem_chunk_init(&chunk[i + 1], lh_start, lh_size, type);
			mem_chunk_init(&chunk[i + 2], lh_end + 1,
				       ch_end - lh_end, ch_type);
			break;
		}
	}
}
