ENTRY(_start)
SECTIONS
{
    . = 0X81000000;
    .text : {
        *(*.text)
    }

    .rodata : {
        *(*.rodata)
    }

    .data : {
        *(*.data)
    }

    .bss : {
        *(*.bss)
    }
}