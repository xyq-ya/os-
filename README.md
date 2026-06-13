# os实验

操作系统课程实验代码（project2 ~ project4）。

| 项目 | 内容 |
|------|------|
| project2 | MBR 引导、GDT/TSS、ring3 切换、`int 0x80` 系统调用 |
| project3 | 可变分区 + ELF 加载器，独立用户程序加载到用户空间 |
| project4 | PIC/PIT 时钟中断、双用户进程、时间片轮转调度 |

## 构建与运行

每个项目目录下：

```bash
make
make run
```

需要：`nasm`、`gcc`（32 位）、`ld`、`qemu-system-i386`。
