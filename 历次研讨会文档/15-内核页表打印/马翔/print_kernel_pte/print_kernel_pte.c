#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <asm/pgtable.h>
#include <asm/current.h>

#define INIT_MM_ADDR 		0xc19082c0
#define SWAPPER_PG_DIR_ADDR 	0xc1a59000

static void walk_pte_table(pte_t *pte_table, unsigned long addr)
{
	int index;
	unsigned long cur_addr;
	
	for(index = 0;index < PTRS_PER_PTE;index ++)
	{
		cur_addr = addr + index * PAGE_SIZE;
		if(pte_present(pte_table[index]))
		{
			printk("\t\t\t|Linear: 0x%lx, Phyical: 0x%08llx\n", cur_addr, (unsigned long long)(pte_val(pte_table[index]) & PAGE_MASK));
		}
	}
}

static void walk_pmd_table(pmd_t *pmd_table, unsigned long addr)
{
	int index;
	unsigned long cur_addr;

	for(index = 0;index < 1 /*PTRS_PER_PMD*/;index ++)
	{
		printk("\t\t|walk_pmd_table, index %d, value 0x%lx\n", index, (unsigned long)pmd_table[index].pmd);
		cur_addr = addr + index * PMD_SIZE;
		if(pmd_present(pmd_table[index]))
		{
			pte_t *pte_table = pte_offset_kernel(pmd_table + index, cur_addr);
			walk_pte_table(pte_table, cur_addr);
		}
	}
}

/*
static void walk_pud_table(pud_t *pud_table, unsigned long addr)
{
	int index;
	unsigned long cur_addr;

	for(index = 0;index < PTRS_PER_PUD;index ++)
	{
		printk("\t|walk_pud_table, index %d, value 0x%lx\n", index, (unsigned long)pud_table[index].pud);
		cur_addr = addr + index * PUD_SIZE;
		if(pud_present(pud_table[index]))
		{
			pmd_t *pmd_table = pmd_offset(pud_table + index, cur_addr);
			walk_pmd_table(pmd_table, cur_addr);
		}
	}
}
*/

static void walk_pgd_table(pgd_t *pgd_table, unsigned long addr)
{
	int index;
	unsigned long cur_addr;
	
	for(index = 3/*0*/;index < PTRS_PER_PGD;index ++)
	{
		printk("|walk_pgd_table, index %d, value 0x%lx\n", index, (unsigned long)pgd_table[index].pgd);
		cur_addr = addr + index * PGDIR_SIZE;
		if(pgd_present(pgd_table[index]))
		{
		//	pud_t *pud_table = pud_offset(pgd_table + index, cur_addr);
		//	walk_pud_table(pud_table, cur_addr);
			pmd_t *pmd_table = pmd_offset((pud_t *)(pgd_table + index), cur_addr);
			walk_pmd_table(pmd_table, cur_addr);
		}
	}
}

static void print_kernel_pte(void)
{	
	pgd_t *pgd_table = current->mm->pgd;	
//	pgd_t *pgd_table = (pgd_t *)SWAPPER_PG_DIR_ADDR;
//	struct mm_struct *init_mm_p = (struct mm_struct *)INIT_MM_ADDR;	
//	pgd_t *pgd_table = init_mm_p->pgd;

	printk("===>>> Printing Kernel PTE Started <<<===\n");
	walk_pgd_table(pgd_table, 0);
	printk("===>>> Printing Kernel PTE Ended <<<===\n");
}

/* [0xc0000000 - 0xf8000000]: Directly Maped , [0xf8000000 - 0xffffffff]: Vmalloc & Permanent & Fixed Maped */
static int __init my_init(void)
{
	printk("===>>> MY_INIT Success <<<===\n");
	print_kernel_pte();
	return 0;
}

static void __exit my_exit(void)
{
	printk("===>>> MY_EXIT Success <<<===\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SuperMXC");
MODULE_DESCRIPTION("print_kernel_pte");
