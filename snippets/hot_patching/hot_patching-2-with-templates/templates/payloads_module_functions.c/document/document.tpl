//Lists of payload 'init' and 'exit' functions
#include <linux/kernel.h>
#include <linux/init.h>

<$payloadInitDecl : join(\n)$>
int (*payloads_init[])(void) __initdata =
{
    <$payloadInit : join(,\n)$>
};

<$payloadExitDecl : join(\n)$>
void (*payloads_exit[])(void) =
{
    <$payloadExit : join(,\n)$>
};

int payloads_n = ARRAY_SIZE(payloads_init);