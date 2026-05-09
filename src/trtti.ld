/* linker script for the correct placement of the rtti section and the symbols in it, which is needed for id generation of types with rtti and creates an unique ID across translation units */
/* docs used:
https://home.cs.colorado.edu/~main/cs1300/doc/gnu/ld_3.html#SEC16
https://wiki.osdev.org/Linker_Scripts
*/
SECTIONS {
    /* define how the rtti section is used */ 
    ._rtti_impl_trtti_lib_section : {
        /* the start symbol comes first */ 
        __impl_global_data_rtti_linker__impl__start = .;
        /* this symbol has to be at the start */
        __impl_global_data_rtti___impl_global_start_postion_for_rtti_ids__id_data = .;
        /* keep the rest no matter what */
        KEEP(*(._rtti_impl_trtti_lib_section))
        /* the end symbol comes last */ 
        __impl_global_data_rtti_linker__impl__end = .;
    }
    /* insert this sections (its just one) after the rodata section*/
} INSERT AFTER .rodata;
 