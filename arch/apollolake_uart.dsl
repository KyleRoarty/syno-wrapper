DefinitionBlock ("", "SSDT", 2, "", "", 0)
{
    External (\_SB.PCI0.URT2.VUT1, DeviceObj)
    Scope (\_SB.PCI0.URT2.VUT1)
    {
        Method (_STA, 0, NotSerialized)
        {
            Return (Zero)
        }
    }

    External (\_SB.PCI0.URT2, DeviceObj)
    Scope (\_SB.PCI0.URT2)
    {
        Device (VUT2)
        {
            Name (_HID, "TST0001")
            Method (_STA, 0, NotSerialized)   // ensure "present/enabled"
            {
                Return (0x0F)
            }
            Method (_CRS, 0, NotSerialized)
            {
                Name(SBUF, ResourceTemplate () {
                    UartSerialBusV2 (0x0001C200, DataBitsEight, StopBitsOne,
                    0xFC, LittleEndian, ParityTypeNone, FlowControlNone,
                    0x0020, 0x0020, "\\_SB.PCI0.URT2",
                    0x00, ResourceConsumer, , Exclusive,
                    )
                })
                Return (SBUF)
            }
        }
    }
}