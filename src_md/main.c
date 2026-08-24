/*
 * SEGA CD Mode 1 Support
 * by Chilly Willy
 */

#include <stdint.h>
#include <string.h>

extern uint32_t vblank_vector;
extern uint16_t gen_lvl2;
extern uint16_t cd_ok;

extern uint32_t Sub_Start;
extern uint32_t Sub_End;

uint32_t cd_sp_init_wait;
uint32_t cd_sp_main_wait;
uint16_t cd_init_failure;

extern void Kos_Decomp(uint8_t *src, uint8_t *dst);

extern void write_byte(unsigned int dst, unsigned char val);
extern void write_word(unsigned int dst, unsigned short val);
extern void write_long(unsigned int dst, unsigned int val);
extern unsigned char read_byte(unsigned int src);
extern unsigned short read_word(unsigned int src);
extern unsigned int read_long(unsigned int src);

extern void do_main(void);

uint16_t InitCD(void)
{
    char *bios;
    uint32_t timeout;
    int sub_program_size = (int)&Sub_End - (int)&Sub_Start;

    cd_sp_init_wait = 0;
    cd_sp_main_wait = 0;
    cd_init_failure = 0;

    /*
     * Check for CD BIOS
     * When a cart is inserted in the MD, the CD hardware is mapped to
     * 0x400000 instead of 0x000000. So the BIOS ROM is at 0x400000, the
     * Program RAM bank is at 0x420000, and the Word RAM is at 0x600000.
     */
    bios = (char *)0x415800;
    if (memcmp(bios + 0x6D, "SEGA", 4))
    {
        bios = (char *)0x416000;
        if (memcmp(bios + 0x6D, "SEGA", 4))
        {
            // Check for WonderMega/X'Eye
            if (memcmp(bios + 0x6D, "WONDER", 6))
            {
                bios = (char *)0x41AD00; // Might also be 0x40D500
                // Check for LaserActive
                if (memcmp(bios + 0x6D, "SEGA", 4))
                {
                    cd_init_failure = 1;
                    return 0; // No CD
                }
            }
        }
    }

    /*
     * Reset the Gate Array - this specific sequence of writes is recognized by
     * the gate array as a reset sequence, clearing the entire internal state -
     * this is needed for the LaserActive
     */
    write_word(0xA12002, 0xFF00);
    write_byte(0xA12001, 0x03);
    write_byte(0xA12001, 0x02);
    write_byte(0xA12001, 0x00);

    /*
     * Reset the Sub-CPU, request the bus
     */
    write_byte(0xA12001, 0x02);
    timeout = 2000000;
    while (!(read_byte(0xA12001) & 2) && timeout)
    {
        write_byte(0xA12001, 0x02); // Wait on bus acknowledge
        timeout--;
    }
    if (!timeout)
    {
        cd_init_failure = 2;
        return 0;
    }

    /*
     * Decompress Sub-CPU BIOS to Program RAM at 0x00000
     */
    write_word(0xA12002, 0x0002); // No write-protection, bank 0, 2M mode, Word RAM assigned to Sub-CPU
    memset((char *)0x420000, 0, 0x20000); // Clear program ram first bank - needed for the LaserActive
    Kos_Decomp((uint8_t *)bios, (uint8_t *)0x420000);

    /*
     * Copy Sub-CPU program to Program RAM at 0x06000
     */
    memcpy((char *)0x426000, (char *)&Sub_Start, sub_program_size);
    if (memcmp((char *)0x426000, (char *)&Sub_Start, sub_program_size))
    {
        cd_init_failure = 5;
        return 0;
    }

    write_byte(0xA1200E, 0x00); // Clear main comm port
    write_byte(0xA12002, 0x2A); // Write-protect up to 0x05400
    write_byte(0xA12001, 0x01); // Clear bus request, deassert reset - allow CD Sub-CPU to run
    timeout = 2000000;
    while (!(read_byte(0xA12001) & 1) && timeout)
    {
        write_byte(0xA12001, 0x01); // Wait on Sub-CPU running
        timeout--;
    }
    if (!timeout)
    {
        cd_init_failure = 2;
        return 0;
    }

    /*
     * Set the vertical blank handler to generate Sub-CPU level 2 ints.
     * The Sub-CPU BIOS needs these in order to run.
     */
    gen_lvl2 = 1; // Generate Level 2 IRQ to Sub-CPU

    /*
     * Wait for Sub-CPU program to set sub comm port indicating it is running -
     * note that unless there's something wrong with the hardware, a timeout isn't
     * needed... just loop until the Sub-CPU program responds, but 2000000 is about
     * ten times what the LaserActive needs, and the LA is the slowest unit to
     * initialize
     */
    while (read_byte(0xA1200F) != 'I')
    {
        cd_sp_init_wait++;
        if (cd_sp_init_wait > 2000000)
        {
            gen_lvl2 = 0;
            cd_init_failure = 3;
            return 0; // No CD
        }
    }

    /*
     * Wait for Sub-CPU to indicate it is ready to receive commands
     */
    while (read_byte(0xA1200F) != 0x00)
    {
        cd_sp_main_wait++;
        if (cd_sp_main_wait > 2000000)
        {
            gen_lvl2 = 0;
            cd_init_failure = 4;
            return 0;
        }
    }

    return 1; // CD ready to go!
}

int main(void)
{
    cd_ok = 0; // InitCD();

    /*
     * Main loop in ram - you need to have it in ram to avoid bus contention
     * for the rom with the SH2s.
     */
    do_main(); // Never returns

    return 0;
}
