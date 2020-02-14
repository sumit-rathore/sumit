#include <leon_uart.h>
#define MY_DEBUG 0x80000010


// Random string.  This was grabbed from the wikipedia page for a novel.

//static const char myString[] = "A novel is a\n";

static const char myString[] = "A novel is a long narrative in literary prose. The genre has historical roots both in the fields of the medieval and early modern romance and in the tradition of the novella. The latter supplied the present generic term in the late 18th century.\n The further definition of the genre is historically difficult. Most of the criteria (such as artistic merit, fictionality, a design to create an epic totality of life, a focus on history and the individual) are arbitrary and designed to raise further debates over qualities that will supposedly separate great works of literature both from a wider and lower \"trivial\" production and from the field of true histories. To become part of the literary production novels have to address the discussion of art. The construction of the narrative, the plot, the way reality is created in the work of fiction, the fascination of the character study, and the use of language are usually discussed to show a novel's artistic merits. Most of these requirements were introduced in the 16th and 17th centuries, in order to give fiction a justification outside the field of factual history. The individualism of the presentation makes the personal memoir and the autobiography the two closest relatives among the genres of modern histories. \r\n";

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32 i;

    // Configure the uart
    LEON_UartSetBaudRate115200();

   //Test the uart code to not overwrite anything
    for (i = 0; i < sizeof(myString); i++)
    {
        LEON_UartByteTx(myString[i]);
    }


    // Wait for everything to be written
    //LEON_UartWaitForTx();

    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
        ;
}
