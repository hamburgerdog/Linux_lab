#define NR_PGT 0x4   //   需要初始化的页面个数
#define PGD_BASE (unsigned int *)0x1000     //  映射到物理内存的地址
#define PAGE_OFFSET (unsigned int)0x2000    //  

#define PTE_PRE 0x01    // 初始化时 页表会装入内存
#define PTE_RW  0x02    // 与U/S位形成硬件保护 
#define PTE_USR 0x04    //  Page Write-Through 写透方式

void page_init(){
    int pages = NR_PGT; //  系统初始化时创建4个页表
    unsigned int page_offset = PAGE_OFFSET;
    unsigned int * pgd = PGD_BASE;          //  页目录表存放的物理地址
    while (pages--)
    {
        * pgd++ = page_offset |PTE_USR|PTE_RW|PTE_PRE;  //  创建四个页目录表
        page_offset += 0x1000   //  每个页目录表的大小为2^12=4KB
    }
    pgd = PGD_BASE;

    //  页表从物理内存第三个页框开始
    unsigned int *pgt_entry = (unsigned int *)0x2000;   
    unsigned int phy_add = 0x0000;
    while (phy_add < 0x1000000) //  0x1000000=16MB 初始化了四个页表，每个页表映射了4MB的物理内存地址
    {
        * pgt_entry = phy_add |PTE_USR|PTE_RW|PTE_PRE;  //  页表项与物理内存真正形成映射
        phy_add += 0x1000;      //  物理块大小和页面大小都是4KB
    }

    //  CR0最高位为控制分页位，linux下分页机制的开启是可选的，则段内嵌汇编的作用就是允许分页
    __asm__ __volatile__("movl  %0, %%cr3;"
                                "movl   %%cr0, %%eax;"
                                "orl    $0x80000000, %%eax;"
                                "movl   %%eax, %%cr0;"::"r"(pgd):"memory","%eax");
    
}