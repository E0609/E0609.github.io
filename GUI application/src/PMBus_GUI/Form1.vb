Imports System.IO
Imports System.Text
Imports System.BitConverter
Imports System.Threading
Imports System.DateTime
Imports Microsoft.Office.Interop.Word
'Imports Microsoft.Office.Interop.Pdf




Public Class Form1

#Region " Variables"

#Region "Common"
    Dim ascii As Encoding = Encoding.ASCII
    Dim Hardware_Selection As Byte = 0
    Dim Hardware_Detected As Byte = 0
    Dim Err_Rec_Count As UInteger = 0
    Dim Poll_Dly_Count As Byte = 0
    Dim Null_Data As Byte
    Dim ReadCaliDataBackCnt As Byte = 0

    Dim Win_I2C_Error As Boolean = False
    Public Pic_Kit_Error As Boolean = False

    Dim RTB1_Line_Num As UInteger = 0
    Public Read_Buf(64) As Byte
    Public Write_Buf(72) As Byte
    Dim Read_Buf_Str As String = ""
    Dim Write_Buf_Str As String = ""
    Dim I2C_Err_Flag As Boolean = False
    Dim Slave_Addr As Byte
    Dim Slave_Addr_Rd As Byte
    Public Poll_Button_Latched As Boolean
    Dim Test_Button_Latched As Boolean
    Dim CRC8_Byte As Byte = 0
    Dim PEC_Err_Flag As Boolean = False
    Dim Query As Boolean = False
    Dim Query_Delay As Byte

    Public index() As Byte = {0, 0} 'cips for index and dataline
    Public Array_c() As Integer = {0, 0} 'cips for index and dataline

#If 0 Then

    Public SlopeL As Double = 0.000
    Public SlopeH As Double = 0.000
    Public OffsetL As Double = 0
    Public OffsetH As Double = 0
    Public ThrL As Double = 0
    Public ThrH As Double = 0
    Public AdcL As Double = 0
    Public AdcLH As Double = 0
#Else
    Public SlopeL As Byte = 0.000
    Public SlopeH As Byte = 0.000
    Public OffsetL As Byte = 0
    Public OffsetH As Byte = 0
    Public ThrL As Byte = 0
    Public ThrH As Byte = 0
    Public AdcL As Byte = 0
    Public AdcH As Byte = 0
    Public ADC_point1 As UInt16 = 0
    Public ADC_point2 As UInt16 = 0
#End If

#End Region

#Region "Pmbus"
    Dim Log_File_Name As String
    Dim FRU_File_Name As String
    Dim Data_Str(3000) As String
    Dim Data_Arr_Pntr As UInteger = 0
    Dim Capture_Data As Boolean = False
    Dim Capture_Data_Delay As UInteger = 0
    Dim Capture_Data_Pntr As UInteger = 0
    Dim ON_OFF_Config As Byte = 0
    Dim PEC_Sta As Boolean = False           'PEC Enabled Status
    Dim Mux_Sta As Boolean = False
    Dim Vout_Resolu As Byte = 21             '24 - 3.90625mV, 25- 7.8125mV, 26 - 15.625mV, 28 - 62.5mV, 29- 125mV
    Dim Pmb_Hex_Data As String = ""
    Dim Pmb_Act_Data As String = ""
    Dim Neg_Str As String = ""
    Dim Neg_T_Sta As Boolean = False
    Dim Page_sel As Byte = 0

    Private Structure PMBus_Rd
        Public Command As Byte
        Public Cmd_Name As String
        Public RW_Length As Byte
        Public Data As Boolean
        Public Vout As Boolean
        Public Resolu_Str As String
        Public Sub New(first As Byte, second As String, third As Byte, fourth As Boolean, fifth As Boolean, Sixth As String)
            Command = first
            Cmd_Name = second
            RW_Length = third
            Data = fourth
            Vout = fifth
            Resolu_Str = Sixth
        End Sub
    End Structure



    Dim PMBus_Data_Struct() As PMBus_Rd = {
     New PMBus_Rd(&H0, "PAGE", 1, False, False, ""),
     New PMBus_Rd(&H78, "STATUS BYTE", 1, False, False, ""),
     New PMBus_Rd(&H79, "STATUS WORD", 2, False, False, ""),
     New PMBus_Rd(&H7A, "STATUS VOUT", 1, False, False, ""),
     New PMBus_Rd(&H7B, "STATUS IOUT", 1, False, False, ""),
     New PMBus_Rd(&H7C, "STATUS INPUT", 1, False, False, ""),
     New PMBus_Rd(&H7D, "STATUS TEMP", 1, False, False, ""),
     New PMBus_Rd(&H7E, "STATUS CML", 1, False, False, ""),
     New PMBus_Rd(&H7F, "STATUS OTHER", 1, False, False, ""),
     New PMBus_Rd(&H80, "STATUS MFR", 1, False, False, ""),
     New PMBus_Rd(&H81, "STATUS_FANS_1_2", 1, False, False, ""),
     New PMBus_Rd(&H88, "READ VIN", 2, True, False, " Volts"),
     New PMBus_Rd(&H89, "READ IIN", 2, True, False, " Amps"),
     New PMBus_Rd(&H8A, "READ VCAP", 2, True, False, " Volts"),
     New PMBus_Rd(&H8B, "READ VOUT", 2, True, True, " Volts"),
     New PMBus_Rd(&H8C, "READ IOUT", 2, True, False, " Amps"),
     New PMBus_Rd(&H8D, "T Ambient", 2, True, False, " 'C"),
     New PMBus_Rd(&H8E, "T Hotspot Sec", 2, True, False, " 'C"),
     New PMBus_Rd(&H8F, "T Hotspot Pri", 2, True, False, " 'C"),
     New PMBus_Rd(&H90, "READ FAN SPEED_1", 2, True, False, " RPM"),
     New PMBus_Rd(&H96, "READ POUT", 2, True, False, " Watts"),
     New PMBus_Rd(&H20, "VOUT_MODE", 1, False, False, ""),
     New PMBus_Rd(&H1, "OPERATION", 1, False, False, ""),
     New PMBus_Rd(&H98, "PMBus Revision", 1, False, False, "")
        }


    Dim PMBus_Cnst_Struct() As PMBus_Rd = {
    New PMBus_Rd(&H19, "CAPABILITY", 1, False, False, ""),
    New PMBus_Rd(&H20, "VOUT_MODE", 1, False, False, ""),
    New PMBus_Rd(&H3A, "FAN_CONFIG_1_2", 1, False, False, ""),
    New PMBus_Rd(&H40, "VOUT_OV_FAULT_LIMIT", 2, True, True, " Volts"),
    New PMBus_Rd(&H46, "IOUT_OC_FAULT_LIMIT", 2, True, False, " Amps"),
    New PMBus_Rd(&H48, "IOUT_OC_LV_FAULT_LIMIT", 2, True, True, " Volts"),
    New PMBus_Rd(&H4A, "IOUT_OC_WARN_LIMIT", 2, True, False, " Amps"),
    New PMBus_Rd(&H4F, "OT_FAULT_LIMIT", 2, True, False, " 'C"),
    New PMBus_Rd(&H51, "OT_WARN_LIMIT", 2, True, False, " 'C"),
    New PMBus_Rd(&H55, "VIN_OV_FAULT_LIMIT ", 2, True, False, " Volts"),
    New PMBus_Rd(&H59, "VIN_UV_FAULT_LIMIT", 2, True, False, " Volts")
       }


    Dim PMBus_MFR_Struct() As PMBus_Rd = {
    New PMBus_Rd(&HA1, "MFR_VIN_MIN", 2, False, False, "Volts"),
    New PMBus_Rd(&HA2, "MFR_VIN_MAX", 2, False, False, "Volts"),
    New PMBus_Rd(&HA3, "MFR_IIN_MAX", 2, False, False, "Amps"),
    New PMBus_Rd(&HA4, "MFR_PIN_MAX", 2, False, False, "Watts"),
    New PMBus_Rd(&HA5, "MFR_VOUT_MAX", 2, True, True, " Volts"),
    New PMBus_Rd(&HA6, "MFR_IOUT_MAX", 2, True, False, " Amps"),
    New PMBus_Rd(&HA7, "MFR_POUT_MAX", 2, True, True, " Watts"),
    New PMBus_Rd(&HA8, "MFR_TAMBIENT_MAX ", 2, True, False, " C"),
    New PMBus_Rd(&HA9, "MFR_TAMBIENT_MIN ", 2, True, False, " 'C"),
    New PMBus_Rd(&HD0, "MFR_FW_REVISION  ", 1, True, False, " ")
       }


    Dim PMBus_LOG_Struct() As PMBus_Rd = {
    New PMBus_Rd(&H79, "STATUS_WORD", 2, False, False, " "),
    New PMBus_Rd(&H7A, "STATUS_VOUT", 2, False, False, "Volts"),
    New PMBus_Rd(&H7B, "STATUS_IOUT", 2, False, False, "Amps"),
    New PMBus_Rd(&H7C, "STATUS_INPUT", 2, False, False, "Watts"),
    New PMBus_Rd(&H7D, "STATUS_TEMPERATURE", 2, True, True, "  C"),
    New PMBus_Rd(&H7E, "STATUS_CML", 2, True, False, "  "),
    New PMBus_Rd(&H80, "STATUS_OTHER", 2, True, True, "  "),
    New PMBus_Rd(&H81, "STATUS_FANS_1_2 ", 2, True, False, "  "),
    New PMBus_Rd(&H88, "READ_VIN ", 2, True, False, "Watts"),
    New PMBus_Rd(&H89, "READ_IIN", 1, True, False, "Amps"),
    New PMBus_Rd(&H8A, "READ_VCAP", 2, False, False, "Volts"),
    New PMBus_Rd(&H8B, "READ_VOUT", 2, False, False, "Volts"),
    New PMBus_Rd(&H8C, "READ_IOUT", 2, False, False, "Amps"),
    New PMBus_Rd(&H8D, "READ_TEMPERATURE_1", 2, False, False, "C"),
    New PMBus_Rd(&H8E, "READ_TEMPERATURE_2", 2, False, False, "C"),
    New PMBus_Rd(&H8F, "READ_TEMPERATURE_3", 2, True, True, " C"),
    New PMBus_Rd(&H90, "READ_FAN_SPEED_1", 2, True, False, " Rpm"),
    New PMBus_Rd(&H96, "READ_POUT ", 2, True, False, "Watts"),
    New PMBus_Rd(&HD0, "MFR_FW_REVISION  ", 1, True, False, " ")
       }

    Private Structure Black_Box_Rd
        Public Command As Byte
        Public Cmd_Name As String
        Public RW_Length As Byte
        Public Data As Boolean
        Public Vout As Boolean
        Public Resolu_Str As String
        Public Sub New(first As Byte, second As String, third As Byte, fourth As Boolean, fifth As Boolean, Sixth As String)
            Command = first
            Cmd_Name = second
            RW_Length = third
            Data = fourth
            Vout = fifth
            Resolu_Str = Sixth
        End Sub
    End Structure



    Dim Black_Box_Struct() As Black_Box_Rd = {
     New Black_Box_Rd(&H0, "Failure_Page", 1, False, False, ""),
     New Black_Box_Rd(&H79, "V1_Status_Word", 2, False, False, ""),
     New Black_Box_Rd(&H7A, "V1_Status_Vout", 1, False, False, ""),
     New Black_Box_Rd(&H7B, "V1_Status_Iout", 1, False, False, ""),
     New Black_Box_Rd(&H7C, "Status_Input", 1, False, False, ""),
     New Black_Box_Rd(&H7D, "Status_Temp", 1, False, False, ""),
     New Black_Box_Rd(&H7E, "Status_CML", 1, False, False, ""),
     New Black_Box_Rd(&H81, "Status_Fans_12", 1, False, False, ""),
     New Black_Box_Rd(&H88, "Read_Vin", 2, True, False, " Volts"),
     New Black_Box_Rd(&H89, "Read_Iin", 2, True, False, " Amps"),
     New Black_Box_Rd(&H8B, "Read_Vout", 2, True, False, " Volts"),
     New Black_Box_Rd(&H8C, "Read_Iout", 2, True, False, " Amps"),
     New Black_Box_Rd(&H8D, "Read_Temp1", 2, True, False, " 'C"),
     New Black_Box_Rd(&H8E, "Read_Temp2", 2, True, False, " 'C"),
     New Black_Box_Rd(&H8F, "Read_Temp3", 2, True, False, " 'C"),
     New Black_Box_Rd(&H90, "Read_FanSpeed1", 2, True, False, " RPM"),
     New Black_Box_Rd(&HD5, "FW_Version", 6, True, False, ""),
     New Black_Box_Rd(&HE5, "MFR_POS_TOTAL", 4, True, False, "Secs"),
     New Black_Box_Rd(&HE6, "MFR_POS_LAST", 4, True, False, "Secs"),
     New Black_Box_Rd(&HD2, "VSB_Status_Vout", 2, False, False, ""),
     New Black_Box_Rd(&HD3, "VSB_Status_Iout", 2, False, False, ""),
     New Black_Box_Rd(&HEE, "Read_Vsb", 2, True, False, " Volts"),
     New Black_Box_Rd(&HEF, "Read_Isb", 2, True, False, " Amps")
       }

#End Region
#Region "Internal Data"
    Dim Int_Data(50) As Byte
    Dim Int_Data_Pntr As Byte = 0
    Dim Cal_Int_Flag = False
    Dim Cal_Sta As Byte = 0
    Dim Cal_Save_Sta As Byte = 0
    Dim Cal_Save_Delay As Byte = 0
    Dim Cal_Pass As Boolean = False
    Dim x1 As Double
    Dim x2 As Double
    Dim y1 As Double
    Dim y2 As Double
    Dim z1 As Double
    Dim z2 As Double
    Dim k As Double
    Dim b As Double = 0
    Dim Temp As Double
    Dim Cal_temp As Integer
    Dim Gain As Integer = 0
    Dim Offset As Integer = 0
    Dim offset_temp As Double = 0
    Dim Gain_Temp As Double = 0
    Dim Threshold As UInteger = 0
    Dim Threshold_Temp As Double = 0
    Dim Neg_Offset As Boolean = False
    Dim PMBus_Factor As Byte = 0
    Dim Pmbus_Rd_Data As UInteger = 0
    Dim RANGE As UInteger = 0

    Private Structure Intern_Data
        Public Addr As Byte
        Public Reg_Name As String
        Public Length As Byte
        Public Data As Boolean
        Public Resol As Single
        Public Resolu_Str As String
        Public Visible As Boolean
        Public Sub New(ByVal first As Byte, ByVal second As String, ByVal third As Byte, ByVal fourth As Boolean, ByVal fifth As Single, ByVal sixth As String, ByVal seventh As Boolean)
            Addr = first
            Reg_Name = second
            Length = third
            Data = fourth
            Resol = fifth
            Resolu_Str = sixth
            Visible = seventh
        End Sub
    End Structure
    Dim Internal_Data_Struct() As Intern_Data = {
        New Intern_Data(&HD5, "Pri. FW Rev.", 2, False, 0.0, "", True),
        New Intern_Data(&HD7, "Sec. FW Rev.", 2, False, 0.0, "", True),
        New Intern_Data(&HE0, "Primary Status 0", 2, False, 0.0, "", True),
        New Intern_Data(&HE1, "Primary Status 1", 2, False, 0.0, "", True),
        New Intern_Data(&HE2, "Secondary Flag 0", 2, False, 0.0, "", True),
        New Intern_Data(&HE3, "Secondary Flag 1", 2, False, 0.0, "", True),
        New Intern_Data(&HE4, "Secondary Flag 2", 2, False, 0.0, "", True),
        New Intern_Data(&HE5, "Secondary V1 Ext", 2, False, 0.0, "", True),
        New Intern_Data(&HF9, "Addr. Pointer Read", 2, False, 0.0, "", True),
        New Intern_Data(&HFA, "Debug Command", 2, False, 0.0, "", True),
        New Intern_Data(&HF5, "PMBus ADR IIN ADC", 2, False, 0.0, "", True),
        New Intern_Data(&HF6, "PMBus ADR VIN ADC", 2, False, 0.0, "", True),
        New Intern_Data(&HEB, "Time Odometer", 3, False, 0.0, "", True),
        New Intern_Data(&HF7, "PMBus ADR PIN ADC", 2, False, 0.0, "", True)
        }

#End Region
#End Region

#Region "Common Controls & Functions"
#Region "Form Controls"

    Private Sub init_EE_display()
        Dim i As Byte = 0
        DataGridView_ee.ColumnCount = 16
        DataGridView_ee.Columns(0).Name = " "
        DataGridView_ee.Columns(1).Name = "ID"
        DataGridView_ee.Columns(2).Name = "Name"
        DataGridView_ee.Columns(3).Name = "Price"


        For i = 0 To 15
            'Read_Buf(i) = 0
            Dim str As String = " "
            Dim str1 As String = " "

            str = Convert.ToString(i)
            Dim row As String() = New String() {str, " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " "}
            DataGridView_ee.Rows.Add(row)
            str1 = Convert.ToString((i), 16)
            DataGridView_ee.Rows.Item(i).HeaderCell.Value = "h" & str1 & "0"

        Next
#If 0 Then

        Dim row As String() = New String() {"1", " ", " "}
        DataGridView_ee.Rows.Add(row)
        DataGridView_ee.Rows.Item(0).HeaderCell.Value = "0x00"

        row = New String() {"2", " ", " "}
        DataGridView_ee.Rows.Add(row)
        DataGridView_ee.Rows.Item(1).HeaderCell.Value = "0x10"

        row = New String() {"3", " ", " "}
        DataGridView_ee.Rows.Add(row)
        DataGridView_ee.Rows.Item(2).HeaderCell.Value = "0x20"

        row = New String() {"4", "Product 4", "4000"}
        DataGridView_ee.Rows.Add(row)
        DataGridView_ee.Rows.Item(3).HeaderCell.Value = "0x30"
#End If


    End Sub


    Private Sub Form1_Load(sender As System.Object, e As System.EventArgs) Handles MyBase.Load
#If remove Then
        ' Init_Pmbus_DGV(0)
        'Init_Pmbus_Cnst_DGV(0)
        '  Init_Pmbus_Cnst_1_DGV(0)
        '  Init_Pmbus_MFR_DGV(0)
        ' Init_Pmbus_Log_DGV(0)
#End If
        Me.WindowState = FormWindowState.Maximized
        Me.RadioButton_Enable.Checked = True
        Me.RadioButton_Disable.Checked = False
        Me.RadioButton_p.Checked = True
        Me.RadioButton_s.Checked = False
        'Me.ListBox_S_Addr.SelectedIndex = 0
        Me.ListBox_Cmd.SelectedIndex = 0
        Me.ListBox_index.SelectedIndex = 0
        'Me.Num_Eeprom_Addr.SelectedIndex = 0
        Me.NumericUpDown_EEPROM_Addr.Value = 164
        ' Me.OnBackgroundImageChangedRadioButton3_CheckedChanged()
        init_EE_display()


    End Sub
    Private Sub Form1_FormClosed(ByVal sender As System.Object, ByVal e As System.Windows.Forms.FormClosedEventArgs) Handles MyBase.FormClosed
        If Hardware_Selection = 1 Then
            ShutdownProcedure()
        ElseIf Hardware_Selection = 2 Then
            PICkitS.Device.Cleanup()
        End If
    End Sub
    Private Sub Button14_Click(sender As System.Object, e As System.EventArgs)
        Init_PKSA(0)
    End Sub
    Private Sub RadioButton17_Click(sender As System.Object, e As System.EventArgs)
        If Hardware_Selection = 1 Then
            RadioButton17.Checked = True
            RadioButton16.Checked = False
            RadioButton17.BackColor = Color.GreenYellow
            RadioButton16.BackColor = Color.Transparent
            SetI2CFrequency(100)
            Append_Text1("WIN-I2C Frequency Set to 100KHz" & vbCrLf)
            Dim Freq As Integer = GetI2CFrequency()
            Append_Text1("I2C Frequency Read as " & Convert.ToString(Freq, 10).ToUpper & " Khz" & vbCrLf)
        End If
    End Sub
    Private Sub RadioButton16_Click(sender As System.Object, e As System.EventArgs)
        If Hardware_Selection = 1 Then
            RadioButton17.Checked = False
            RadioButton16.Checked = True
            RadioButton17.BackColor = Color.Transparent
            RadioButton16.BackColor = Color.GreenYellow
            SetI2CFrequency(400)
            Append_Text1("WIN-I2C Frequency Set to 400KHz" & vbCrLf)
            Dim Freq As Integer = GetI2CFrequency()
            Append_Text1("I2C Frequency Read as " & Convert.ToString(Freq, 10) & " Khz" & vbCrLf)
        End If
    End Sub
    Private Sub Poll_Click(sender As System.Object, e As System.EventArgs)
        If Poll_Button_Latched = False Then
            Poll_Button_Latched = True
            Poll.BackColor = Color.GreenYellow
        Else
            Poll_Button_Latched = False
            Poll.BackColor = Color.Transparent
        End If
    End Sub


    Private Sub ReadOnce_Click(sender As System.Object, e As System.EventArgs)
        If Pic_Kit_Error = False Then
            Read_All(0)
        End If
    End Sub
    Private Sub CheckBox1_CheckedChanged(sender As System.Object, e As System.EventArgs)
        If CheckBox1.Checked = True Then
            PEC_Sta = True
        Else
            PEC_Sta = False
        End If
    End Sub
#End Region
#Region "Functions"

    Private Sub Sel_Mux_Ch(ByVal Mux As Byte, ByVal Channel As Byte)
        Dim Return_Str As String = ""
        Dim Addr As Byte
        If Mux = 0 Then
            Addr = &HE0
        ElseIf Mux = 1 Then
            Addr = &HE2
        ElseIf Mux = 2 Then
            Addr = &HE6
        ElseIf Mux = 3 Then
            Addr = &HE4
        End If

        If Channel = 1 Then
            Write_Buf(0) = &H1
        ElseIf Channel = 2 Then
            Write_Buf(0) = &H2
        ElseIf Channel = 3 Then
            Write_Buf(0) = &H4
        ElseIf Channel = 4 Then
            Write_Buf(0) = &H8
        ElseIf Channel = 5 Then
            Write_Buf(0) = &H10
        ElseIf Channel = 6 Then
            Write_Buf(0) = &H20
        ElseIf Channel = 7 Then
            Write_Buf(0) = &H40
        ElseIf Channel = 8 Then
            Write_Buf(0) = &H80
        Else
            Write_Buf(0) = 0
        End If

        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "

        If Hardware_Selection = 1 Then
            If Win_I2C_Error = False Then
                Dim Write_Sta As Byte = I2CWriteArray(Addr, Write_Buf(0), 0, Write_Buf(0))
                If Write_Sta = 0 Then
                    ' successful, display results     
                    Append_Text1("Write Byte Sucessful - " & Convert.ToString(Write_Buf(0), 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Win_I2C_Error = True
                    Write_Buf(0) = 0
                End If
            End If
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Addr, Write_Buf(0), 0, Write_Buf, Return_Str)) Then
                    ' successful, display results     
                    Append_Text1("Write Byte Sucessful - " & Convert.ToString(Write_Buf(0), 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Write_Buf(0) = 0
                End If
            End If
        End If

        Return_Str = "-"
    End Sub
    Public Function Init_PKSA(ByVal Null_Data As Byte)
        Dim Status As Boolean = False
#If 1 Then
        If (PICkitS.Device.Initialize_PICkitSerial()) Then
            If (PICkitS.I2CM.Configure_PICkitSerial_For_I2CMaster()) Then

                PICkitS.Device.Reset_Control_Block()
                PICkitS.Device.Clear_Comm_Errors()
                PICkitS.Device.Clear_Status_Errors()

                PICkitS.I2CM.Set_I2C_Bit_Rate(100)

                PICkitS.I2CM.Set_Pullup_State(True)

                PICkitS.I2CM.Set_Aux2_Direction(False)   'set as Output
                Thread.Sleep(200)

                PICkitS.I2CM.Set_Aux2_State(True)   'set as high 
                Thread.Sleep(200)
                PICkitS.I2CM.Set_Aux2_State(False)   'set as low

                Thread.Sleep(200)
                PICkitS.I2CM.Set_Aux2_State(True)   'set as high 
                Thread.Sleep(200)

                Append_Text1("PICkit Serial Analyzer Detected & correctly configured for I2C" & vbCrLf)
            Else
                Append_Text1("Error configuring PICkit Serial for I2C" & vbCrLf)
                PICkitS.Device.Reset_Control_Block()
            End If
            Status = True
        Else
            Append_Text1("Error initializing PICkit Serial" & vbCrLf)
            PICkitS.Device.Reset_Control_Block()
        End If
#Else
        Append_Text1("PICkit Serial Analyzer Detected & correctly configured for I2C" & vbCrLf)
        Status = True
#End If
        Return Status
    End Function
    Public Sub Append_Text1(ByVal Str_Data As String)
        If (RTB1_Line_Num > 1000) Then
            RichTextBox1.ReadOnly = True
            RTB1_Line_Num = 0
            RichTextBox1.Text = ""
        Else
            RichTextBox1.AppendText(Str_Data)
            RichTextBox1.SelectionStart = RichTextBox1.Text.Length
            RichTextBox1.ScrollToCaret()
        End If
        RTB1_Line_Num += 1
    End Sub
    Public Sub Read_All(ByVal Null_Data As Byte)
        Dim TabName As String = TabControl1.SelectedTab.Name
        If TabName = "Tab_Read" Then
            Append_Text1("Started to Read PMBus Status Data........" & vbCrLf)
            Update_Pmbus_Data(0)
            'Send_Byte(&H3)
        End If
    End Sub
    Public Function Read_Byte(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = "00 00"



        'Slave_Addr = NumericUpDown1.Value
        If NumericUpDown_S_Addr.Value = 1 Then
            Slave_Addr = &HB4
        Else
            Slave_Addr = &HB6
        End If



        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"

        If Hardware_Selection = 1 Then
            If Win_I2C_Error = False Then
                Dim Read_Sta As Byte = I2CReadArray(Slave_Addr, Cmd_Addr, 2, Read_Buf(0))
                If Read_Sta = 0 Then
                    Return_Str = Process_Byte(Cmd_Addr)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Win_I2C_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, 2, Read_Buf, Return_Str)) Then
                    Return_Str = Process_Byte(Cmd_Addr)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        End If
        Return Return_Str
    End Function


    Public Function Process_Byte(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = "00 00"
        Dim Return_Str_Buf As String = ""
        ' successful, display results
        'Verify CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)

        Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Pmb_Hex_Data = Return_Str
        Return_Str = Return_Str & " "

        Return_Str_Buf = Convert.ToString(Read_Buf(1), 16).ToUpper
        If Not Return_Str_Buf.Length = 2 Then
            Return_Str_Buf = "0" & Return_Str_Buf
        End If
        Return_Str = Return_Str & Return_Str_Buf

        If CRC8_Byte = Read_Buf(1) Or PEC_Sta = False Then
            Append_Text1("Read Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Return_Str & vbCrLf)
        Else
            Append_Text1("Read Byte PEC Error - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Return_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
            PEC_Err_Flag = True
        End If
        Return Return_Str
    End Function
    Public Function Read_Word(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = "00 00 00"
        Dim Return_Str_Buf1 As String = ""
        Dim Return_Str_Buf2 As String = ""
        Slave_Addr = NumericUpDown_S_Addr.Value

        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"
        If Hardware_Selection = 1 Then
            If Win_I2C_Error = False Then
                Dim Read_Sta As Byte = I2CReadArray(Slave_Addr, Cmd_Addr, 3, Read_Buf(0))
                If Read_Sta = 0 Then
                    Return_Str = Process_Word(Cmd_Addr)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Win_I2C_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, 3, Read_Buf, Return_Str)) Then
                    Return_Str = Process_Word(Cmd_Addr)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        End If
        Return Return_Str
    End Function


    Public Function Read_Linear_Word_Pmb(ByVal Cmd_Addr As Byte, ByVal Array_Loc As Byte)
        Dim Return_Str As String = "00 00 00"
        Dim Return_Str_Buf1 As String = ""
        Dim Return_Str_Buf2 As String = ""
        Dim Data_Float As Single = 0.0
        Dim IntConverter As Integer

        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"

        Slave_Addr = NumericUpDown_S_Addr.Value

        If Hardware_Selection = 1 Then
            If Win_I2C_Error = False Then
                Dim Read_Sta As Byte = I2CReadArray(Slave_Addr, Cmd_Addr, 3, Read_Buf(0))
                If Read_Sta = 0 Then
                    Return_Str = Process_Word(Cmd_Addr)

                    If Not PMBus_Data_Struct(Array_Loc).Vout = True Then '11 bit Conversion
                        Neg_Str = ""
                        If PMBus_Data_Struct(Array_Loc).Command <= &H8F And PMBus_Data_Struct(Array_Loc).Command >= &H8D Then 'For Negative Temperature
                            Neg_T_Sta = True
                        Else
                            Neg_T_Sta = False
                        End If

                        Data_Float = Lin11_2_Float(Read_Buf)
                        Data_Float = Data_Float * 1000
                        IntConverter = Convert.ToDecimal(Data_Float)
                        Data_Float = Convert.ToSingle(IntConverter)
                        Data_Float = Data_Float / 1000
                        Pmb_Act_Data = Neg_Str & Convert.ToString(Data_Float) & PMBus_Data_Struct(Array_Loc).Resolu_Str
                    Else    '16 Bit Conversion
                        Data_Float = Lin16_2_Float(Read_Buf)
                        Data_Float = Data_Float * 1000
                        IntConverter = Convert.ToDecimal(Data_Float)
                        Data_Float = Convert.ToSingle(IntConverter)
                        Data_Float = Data_Float / 1000
                        Pmb_Act_Data = Convert.ToString(Data_Float) & PMBus_Data_Struct(Array_Loc).Resolu_Str
                    End If
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Win_I2C_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, 3, Read_Buf, Return_Str)) Then
                    Return_Str = Process_Word(Cmd_Addr)

                    If PMBus_Data_Struct(Array_Loc).Vout = False Then '11 bit Conversion
                        Neg_Str = ""
                        If PMBus_Data_Struct(Array_Loc).Command <= &H8F And PMBus_Data_Struct(Array_Loc).Command >= &H8D Then 'For Negative Temperature
                            Neg_T_Sta = True
                        Else
                            Neg_T_Sta = False
                        End If

                        Data_Float = Lin11_2_Float(Read_Buf)
                        Data_Float = Data_Float * 1000
                        IntConverter = Convert.ToDecimal(Data_Float)
                        Data_Float = Convert.ToSingle(IntConverter)
                        Data_Float = Data_Float / 1000
                        Pmb_Act_Data = Neg_Str & Convert.ToString(Data_Float) & PMBus_Data_Struct(Array_Loc).Resolu_Str
                    Else    '16 Bit Conversion
                        Data_Float = Lin16_2_Float(Read_Buf)
                        Data_Float = Data_Float * 1000
                        IntConverter = Convert.ToDecimal(Data_Float)
                        Data_Float = Convert.ToSingle(IntConverter)
                        Data_Float = Data_Float / 1000
                        Pmb_Act_Data = Convert.ToString(Data_Float) & PMBus_Data_Struct(Array_Loc).Resolu_Str
                    End If
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        End If
        Return Return_Str
    End Function
    Public Function Read_Linear_Word_Cnst(ByVal Cmd_Addr As Byte, ByVal Array_Loc As Byte)
        Dim Return_Str As String = "00 00 00"
        Dim Return_Str_Buf1 As String = ""
        Dim Return_Str_Buf2 As String = ""
        Dim Data_Float As Single = 0.0
        Dim IntConverter As Integer

        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"

        Slave_Addr = NumericUpDown_S_Addr.Value

        If Hardware_Selection = 1 Then
            If Win_I2C_Error = False Then
                Dim Read_Sta As Byte = I2CReadArray(Slave_Addr, Cmd_Addr, 3, Read_Buf(0))
                If Read_Sta = 0 Then
                    Return_Str = Process_Word(Cmd_Addr)

                    If Not PMBus_Cnst_Struct(Array_Loc).Vout = True Then '11 bit Conversion
                        Neg_Str = ""
                        If PMBus_Cnst_Struct(Array_Loc).Command = &HA9 Then 'For Negative Temperature
                            Neg_T_Sta = True
                        Else
                            Neg_T_Sta = False
                        End If

                        Data_Float = Lin11_2_Float(Read_Buf)
                        Data_Float = Data_Float * 1000
                        IntConverter = Convert.ToDecimal(Data_Float)
                        Data_Float = Convert.ToSingle(IntConverter)
                        Data_Float = Data_Float / 1000
                        Pmb_Act_Data = Neg_Str & Convert.ToString(Data_Float) & PMBus_Cnst_Struct(Array_Loc).Resolu_Str
                    Else    '16 Bit Conversion
                        Data_Float = Lin16_2_Float(Read_Buf)
                        Data_Float = Data_Float * 1000
                        IntConverter = Convert.ToDecimal(Data_Float)
                        Data_Float = Convert.ToSingle(IntConverter)
                        Data_Float = Data_Float / 1000
                        Pmb_Act_Data = Convert.ToString(Data_Float) & PMBus_Cnst_Struct(Array_Loc).Resolu_Str
                    End If
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Win_I2C_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, 3, Read_Buf, Return_Str)) Then
                    Return_Str = Process_Word(Cmd_Addr)

                    If PMBus_Cnst_Struct(Array_Loc).Vout = False Then '11 bit Conversion
                        Neg_Str = ""
                        If PMBus_Cnst_Struct(Array_Loc).Command = &HA9 Then 'For Negative Temperature
                            Neg_T_Sta = True
                        Else
                            Neg_T_Sta = False
                        End If

                        Data_Float = Lin11_2_Float(Read_Buf)
                        Data_Float = Data_Float * 1000
                        IntConverter = Convert.ToDecimal(Data_Float)
                        Data_Float = Convert.ToSingle(IntConverter)
                        Data_Float = Data_Float / 1000
                        Pmb_Act_Data = Neg_Str & Convert.ToString(Data_Float) & PMBus_Cnst_Struct(Array_Loc).Resolu_Str
                    Else    '16 Bit Conversion
                        Data_Float = Lin16_2_Float(Read_Buf)
                        Data_Float = Data_Float * 1000
                        IntConverter = Convert.ToDecimal(Data_Float)
                        Data_Float = Convert.ToSingle(IntConverter)
                        Data_Float = Data_Float / 1000
                        Pmb_Act_Data = Convert.ToString(Data_Float) & PMBus_Cnst_Struct(Array_Loc).Resolu_Str
                    End If
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        End If
        Return Return_Str
    End Function
    Public Function Process_Word(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = "00 00 00"
        Dim Return_Str_Buf1 As String = ""
        Dim Return_Str_Buf2 As String = ""

        ' successful, display results
        'Verify CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

        Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        '  Return_Str = Return_Str & " "

        Return_Str_Buf1 = Convert.ToString(Read_Buf(1), 16).ToUpper
        If Not Return_Str_Buf1.Length = 2 Then
            Return_Str_Buf1 = "0" & Return_Str_Buf1
        End If

        Return_Str_Buf2 = Convert.ToString(Read_Buf(2), 16).ToUpper
        If Not Return_Str_Buf2.Length = 2 Then
            Return_Str_Buf2 = "0" & Return_Str_Buf2
        End If
        Pmb_Hex_Data = Return_Str_Buf1 & Return_Str

        Return_Str = Return_Str & " " & Return_Str_Buf1 & " " & Return_Str_Buf2

        If CRC8_Byte = Read_Buf(2) Or PEC_Sta = False Then
            Append_Text1("Read Word Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Return_Str & vbCrLf)
        Else
            ' Append_Text1("Read Word PEC Error - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Return_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
            PEC_Err_Flag = True
        End If

        Return Return_Str
    End Function


    Public Function Lin11_2_Float(ByVal Byte_Array() As Byte)
        Dim Data_Float As Single = 0.0
        Dim Data_Int1 As UInteger = 0
        Dim Data_Int2 As UInteger = 0
        Dim Data_Int3 As Short = 0

        Data_Int1 = Byte_Array(1)
        Data_Int1 *= 256
        Data_Int1 += Byte_Array(0)

        Data_Int2 = Data_Int1 And &H7FF

        If Neg_T_Sta = True And Data_Int2 > &H3FF Then
            Data_Int2 = Not Data_Int2
            Data_Int2 = Data_Int2 + 1
            Data_Int2 = Data_Int2 And &H3FF
            Neg_Str = "-"
        End If

        Data_Int1 = Data_Int1 And &HF800
        Data_Int1 = Data_Int1 >> 11
        If (Data_Int1 > 15) Then
            Data_Int3 = Data_Int1 - 32
        Else
            Data_Int3 = Data_Int1
        End If
        Data_Float = 2 ^ Data_Int3

        Data_Float = Data_Float * Data_Int2

        If PMBus_Data_Struct(15).Command = &H8C Then
            PMBus_Data_Struct(15).Command = &H8C
        End If
        Return Data_Float
    End Function
    Public Function Lin16_2_Float(ByVal Byte_Array() As Byte)
        Dim Data_Float As Single = 0.0
        Dim Data_Int1 As UInteger = 0
        Dim Data_Int3 As Short = 0

        Data_Int1 = Byte_Array(1)
        Data_Int1 *= 256
        Data_Int1 += Byte_Array(0)

        Data_Int3 = Vout_Resolu
        If (Data_Int3 > 15) Then
            Data_Int3 = Data_Int3 - 32
        End If
        Data_Float = 2 ^ Data_Int3

        Data_Float = Data_Float * Data_Int1

        Return Data_Float
    End Function
    Public Function Read_Block(ByVal Cmd_Addr As Byte, ByVal Byte_Count As Byte) ' Read With Length Byte - Count Including PEC 
        Dim Return_Str As String = "00"
        Slave_Addr = NumericUpDown_S_Addr.Value

        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"

        Byte_Count = Byte_Count + 2

        If Hardware_Selection = 1 Then 'Win_I2C_Error check
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, Byte_Count, Read_Buf, Return_Str)) Then
                    Return_Str = Process_Block(Cmd_Addr, Byte_Count)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    For i = 0 To Byte_Count - 1
                        Read_Buf(i) = 0
                    Next
                End If
            End If
        End If


        Return Return_Str
    End Function

    Public Function Read_Block1(ByVal Cmd_Addr As Byte, ByVal Byte_Count As Byte) ' Read With Length Byte - Count Including PEC 
        Dim Return_Str As String = "00"
        Slave_Addr = NumericUpDown_S_Addr.Value

        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"

        Byte_Count = Byte_Count + 2

        If Hardware_Selection = 1 Then
            If Win_I2C_Error = False Then
                Dim Read_Sta As Byte = I2CReadArray(Slave_Addr, Cmd_Addr, Byte_Count, Read_Buf(0))
                If Read_Sta = 0 Then
                    Return_Str = Process_Block(Cmd_Addr, Byte_Count)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Win_I2C_Error = True
                    For i = 0 To Byte_Count - 1
                        Read_Buf(i) = 0
                    Next
                    Read_Buf_Str = ""
                End If
            End If
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, Byte_Count, Read_Buf, Return_Str)) Then
                    Return_Str = Process_Block(Cmd_Addr, Byte_Count)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    For i = 0 To Byte_Count - 1
                        Read_Buf(i) = 0
                    Next
                    Read_Buf_Str = ""
                End If
            End If
        End If
        Return Return_Str
    End Function
    Public Function Read_Block_2(ByVal Cmd_Addr As Byte, ByVal Byte_Count As Byte) ' Read With Length Byte - Count Including PEC 
        Dim Return_Str As String = "00"
        Slave_Addr = NumericUpDown_S_Addr.Value

        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"

        Byte_Count = Byte_Count + 1 ' without length byte

        If Hardware_Selection = 1 Then
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, Byte_Count, Read_Buf, Return_Str)) Then

                    Return_Str = Process_Block(Cmd_Addr, Byte_Count)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    For i = 0 To Byte_Count - 1
                        Read_Buf(i) = 0
                    Next
                End If
            End If
        End If
        Return Return_Str
    End Function
    Public Function Process_Block(ByVal Cmd_Addr As Byte, ByVal Byte_Count As Byte)
        Dim Return_Str As String = "00"
        'successful, display results
        'Verify CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)
        Read_Buf_Str = ""


        For i = 0 To Byte_Count - 2
            CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(i), CRC8_Byte)
            Return_Str = Convert.ToString(Read_Buf(i), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Read_Buf_Str = Read_Buf_Str & Return_Str & " "
        Next

        Return_Str = Convert.ToString(Read_Buf(Byte_Count - 1), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Read_Buf_Str = Read_Buf_Str & Return_Str & " "

        If CRC8_Byte = Read_Buf(Byte_Count - 1) Or PEC_Sta = False Then
            Append_Text1("Read Block Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Read_Buf_Str & vbCrLf)
        Else
            Append_Text1("Read Block PEC Error - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
            PEC_Err_Flag = True
        End If


        Return Return_Str
    End Function
    Private Sub Send_Byte(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = ""
        Slave_Addr = NumericUpDown_S_Addr.Value
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        Write_Buf(0) = CRC8_Byte
        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "

        Dim Data_Len As Byte = 0
        If PEC_Sta = True Then
            Data_Len = 1
        Else
            Data_Len = 0
        End If

        If Hardware_Selection = 1 Then

        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf, Return_Str)) Then
                    ' successful, display results      
                    Append_Text1("Send Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Write_Buf(0) = 0
                    Write_Buf(1) = 0
                    Write_Buf(2) = 0
                End If
            End If
        End If
        Return_Str = "-"
    End Sub
    Private Sub Write_Byte(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = ""
        Slave_Addr = NumericUpDown_S_Addr.Value
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        Write_Buf(1) = CRC8_Byte
        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "
        Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str & " "

        Dim Data_Len As Byte = 0
        If PEC_Sta = True Then
            Data_Len = 2
        Else
            Data_Len = 1
        End If

        If Hardware_Selection = 1 Then

        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf, Return_Str)) Then
                    ' successful, display results     
                    Append_Text1("Write Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Write_Buf(0) = 0
                    Write_Buf(1) = 0
                    Write_Buf(2) = 0
                End If
            End If
        End If

        Return_Str = "-"
    End Sub
    Public Sub Write_Word(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = ""
        Slave_Addr = NumericUpDown_S_Addr.Value
        'Append_Text1(vbCrLf & "Slave_Addr:" & Convert.ToString(Hex(Slave_Addr)) & vbCrLf)
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        Write_Buf(2) = CRC8_Byte
        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "
        Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str & " "
        Return_Str = Convert.ToString(Write_Buf(2), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str & " "

        Dim Data_Len As Byte = 0
        If PEC_Sta = True Then
            Data_Len = 3
        Else
            Data_Len = 2
        End If

        If Hardware_Selection = 1 Then
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf, Return_Str)) Then
                    ' successful, display results     
                    Append_Text1("Write Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Write_Buf(0) = 0
                    Write_Buf(1) = 0
                    Write_Buf(2) = 0
                End If
            End If
        End If
        Return_Str = "-"
    End Sub
    Public Sub Write_Block(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = ""
        Dim i As Integer = 0
        Slave_Addr = NumericUpDown_S_Addr.Value '&H64
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte) '&HFC
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        For i = 1 To Write_Buf(0)
            CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(i), CRC8_Byte)
        Next
        Write_Buf(i) = CRC8_Byte

        ' successful, display results      
        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "

        For i = 1 To Write_Buf(0)
            Return_Str = Convert.ToString(Write_Buf(i), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "
        Next

        Return_Str = Convert.ToString(Write_Buf(i), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str

        Dim Data_Len As Byte = 0
        If PEC_Sta = True Then
            Data_Len = Write_Buf(0) + 2
        Else
            Data_Len = Write_Buf(0) + 1
        End If

        If Hardware_Selection = 1 Then

        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf, Return_Str)) Then
                    Append_Text1("Write Block Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    For i = 0 To Write_Buf(0)
                        Write_Buf(i) = 0
                    Next
                End If
            End If
        End If

        Return_Str = "-"
    End Sub

    Private Sub Write_Block_1(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = ""
        Dim i As Integer = 0
        Slave_Addr = NumericUpDown_S_Addr.Value
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)

        For i = 1 To Write_Buf(0)
            CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(i), CRC8_Byte)
        Next
        Write_Buf(i) = CRC8_Byte

        ' successful, display results      
        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "

        For i = 1 To Write_Buf(0)
            Return_Str = Convert.ToString(Write_Buf(i), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "
        Next

        Return_Str = Convert.ToString(Write_Buf(i), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str

        Dim Data_Len As Byte = 0

        Data_Len = Write_Buf(0) + 2

        If Hardware_Selection = 1 Then
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf, Return_Str)) Then
                    Append_Text1("Write Block Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    For i = 0 To Write_Buf(0)
                        Write_Buf(i) = 0
                    Next
                End If
            End If
        End If

        Return_Str = "-"
    End Sub

    Public Sub LinearFmt_YtoX(ByVal Y As UInt16, ByVal Gain As Byte)



        Dim result As UInt64 = 0
        Dim N As Byte = 0


        N = ((Y & &HF800) >> 11)

        If (Y And &H8000) Then

            result = ((Y & &H7FF) << Gain) >> (32 - N)

        Else

            result = ((Y & &H7FF) << N) << Gain

        End If

        'Return result;

    End Sub
#End Region
#Region "cips new function"
    'Addr_w	0xFA	Index	Addr_r	AdcL	AdcH
    Public Function Process_Word_ADC(ByVal Cmd_Addr As Byte)
        'Addr_w	0xFA	Index	Addr_r	AdcL	AdcH
        Dim Return_Str As String = "00 00 00"
        Dim Return_Str_Buf1 As String = ""
        Dim Return_Str_Buf2 As String = ""

        ' successful, display results
        'Verify CRC 8 
        Write_Buf(0) = Get_Index_adc()
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)

        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

        Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        '  Return_Str = Return_Str & " "

        Return_Str_Buf1 = Convert.ToString(Read_Buf(1), 16).ToUpper
        If Not Return_Str_Buf1.Length = 2 Then
            Return_Str_Buf1 = "0" & Return_Str_Buf1
        End If

        Return_Str_Buf2 = Convert.ToString(Read_Buf(2), 16).ToUpper
        If Not Return_Str_Buf2.Length = 2 Then
            Return_Str_Buf2 = "0" & Return_Str_Buf2
        End If
        Pmb_Hex_Data = Return_Str_Buf1 & Return_Str

        Return_Str = Return_Str & " " & Return_Str_Buf1 & " " & Return_Str_Buf2

        If CRC8_Byte = Read_Buf(2) Or PEC_Sta = False Then
            Append_Text1("Read ADC Word Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Return_Str & vbCrLf)
            Append_Text1("ADC RAW Data:" & Convert.ToString(Read_Buf(1), 16).ToUpper &
                         -" " & Convert.ToString(Read_Buf(0), 16).ToUpper & vbCrLf)
        Else
            ' Append_Text1("Read Word PEC Error - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Return_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
            PEC_Err_Flag = True
        End If

        Return Return_Str
    End Function

    Public Function Read_Word_ADC(ByVal Cmd_Addr As Byte)
        'Addr_w	0xFA	Index	Addr_r	AdcL	AdcH
        Dim Return_Str As String = "00 00 00"
        Dim Return_Str_Buf1 As String = ""
        Dim Return_Str_Buf2 As String = ""
        Slave_Addr = NumericUpDown_S_Addr.Value

        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"
        Write_Buf(0) = Get_Index_adc()
        If Hardware_Selection = 1 Then
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                ' PICkitS.I2CM.

                If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, Write_Buf(0), 3, Read_Buf, Return_Str)) Then
                    Return_Str = Process_Word_ADC(Cmd_Addr)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Read_Buf(0) = 0
                    Read_Buf(1) = 0
                    Read_Buf(2) = 0
                End If
            End If
        End If
        Return Return_Str
    End Function


    Public Sub Write_Block_ADC(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = ""
        Dim i As Integer = 0
        Slave_Addr = NumericUpDown_S_Addr.Value '&H64
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte) '&HFC
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        For i = 1 To Write_Buf(0)
            CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(i), CRC8_Byte)
        Next
        Write_Buf(i) = CRC8_Byte

        ' successful, display results      
        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "

        For i = 1 To Write_Buf(0)
            Return_Str = Convert.ToString(Write_Buf(i), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "
        Next

        Return_Str = Convert.ToString(Write_Buf(i), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str

        Dim Data_Len As Byte = 0
        If PEC_Sta = True Then
            Data_Len = Write_Buf(0) + 2
        Else
            Data_Len = Write_Buf(0) + 1
        End If

        If Hardware_Selection = 1 Then

        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf, Return_Str)) Then
                    Append_Text1("Write Block Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    For i = 0 To Write_Buf(0)
                        Write_Buf(i) = 0
                    Next
                End If
            End If
        End If

        Return_Str = "-"
    End Sub
    Public Function Process_Block_calibration(ByVal Cmd_Addr As Byte, ByVal Byte_Count As Byte) 'cips
        'Addr_w	0xC9	0x02	Index	Dataline	Addr_r	0x06	SlopeL	SlopeH OffsetL OffsetH	ThrL	ThrH

        Dim Return_Str As String = "00"
        'Dim Byte_Count As Byte = 6
#If 0 Then
        Write_Buf(0) = 2
        Write_Buf(1) = index(0)
        Write_Buf(2) = index(1)
#End If
        ' successful, display results
        'Verify CRC 8 

        CRC8_Byte = 0
#If 0 Then
        Read_Buf(0) = 6
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
#End If

        CRC8_Byte = PICkitS.Utilities.calculate_crc8(NumericUpDown_S_Addr.Value + 1, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
        Read_Buf_Str = ""


        For i = 1 To Byte_Count
            CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(i), CRC8_Byte)
            Return_Str = Convert.ToString(Read_Buf(i), 16).ToUpper

            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Read_Buf_Str = Read_Buf_Str & Return_Str & " "
        Next

        Return_Str = Convert.ToString(Read_Buf(Byte_Count - 1), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Read_Buf_Str = Read_Buf_Str & Return_Str & " "

        If CRC8_Byte = Read_Buf(Byte_Count - 1) Or PEC_Sta = False Then
            Append_Text1(" calibration Read Block Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Read_Buf_Str & vbCrLf)
        Else
            Append_Text1("calibration Read Block PEC Error - " & Convert.ToString(Cmd_Addr, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
            PEC_Err_Flag = True
        End If


        Return Return_Str
    End Function
    Public Function Read_Block_calibration(ByVal Cmd_Addr As Byte, ByVal Byte_Count As Byte)
        Dim Return_Str As String = "00"
        Slave_Addr = NumericUpDown_S_Addr.Value

        'Write_Buf(0) = 2
        'Write_Buf(1) = index(0)
        ' Write_Buf(2) = index(1)
        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"

        Byte_Count = Byte_Count + 1 ' without length byte

        If Hardware_Selection = 1 Then
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                'write 
                Write_Buf(0) = 2
                Write_Buf(1) = index(0)
                Write_Buf(2) = index(1)
                Write_Block_ADC(Cmd_Addr)


                'If (PICkitS.I2CM.Read(Slave_Addr, Cmd_Addr, Byte_Count, Read_Buf, Return_Str)) Then
                If (PICkitS.I2CM.Receive(NumericUpDown_S_Addr.Value + 1, Byte_Count, Read_Buf, Return_Str)) Then
                    Return_Str = Process_Block_calibration(Cmd_Addr, Byte_Count)
                Else
                    Append_Text1("Error Reading Data From the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    For i = 0 To Byte_Count - 1
                        Read_Buf(i) = 0
                    Next
                End If
            End If

        End If
        Return Return_Str
    End Function

    Public Sub Write_DWord(ByVal Cmd_Addr As Byte) 'cips
        Dim Return_Str As String = ""
        Slave_Addr = NumericUpDown_S_Addr.Value
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        Write_Buf(4) = CRC8_Byte

        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "

        Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str & " "

        Return_Str = Convert.ToString(Write_Buf(2), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str & " "

        Dim Data_Len As Byte = 0
        If PEC_Sta = True Then
            Data_Len = 3
        Else
            Data_Len = 2
        End If

        If Hardware_Selection = 1 Then
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf, Return_Str)) Then
                    ' successful, display results     
                    Append_Text1("Write Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    Write_Buf(0) = 0
                    Write_Buf(1) = 0
                    Write_Buf(2) = 0
                    Write_Buf(3) = 0
                    Write_Buf(4) = 0
                End If
            End If
        End If
        Return_Str = "-"
    End Sub
    Private Sub Write_Block_calibration(ByVal Cmd_Addr As Byte, ByVal Cmd_Addr_dataline As Byte)
        'Addr_w	0xC9	0x08	Index	Dataline	SlopeL	SlopeH	OffsetL	OffsetH	ThrL	ThrH
        Dim Return_Str As String = ""
        Dim i As Integer = 0
        Dim Index_Length As Byte = 0

        Slave_Addr = NumericUpDown_S_Addr.Value '&H64
        Write_Buf(0) = 8
        Write_Buf(1) = Cmd_Addr               'Get_Index()
        Write_Buf(2) = Cmd_Addr_dataline      'Get_DataLine() 'Cmd_Addr

        Write_Buf(3) = SlopeL 'Convert.ToByte(TextBox_G0.Text) Mod 256
        Write_Buf(4) = SlopeH 'Convert.ToByte(TextBox_G0.Text) \ 256

        Write_Buf(5) = OffsetL 'Convert.ToByte(TextBox_offset0.Text)
        Write_Buf(6) = OffsetH ' Convert.ToByte(TextBox_offset0.Text)

        Write_Buf(7) = ThrL 'Convert.ToByte(TextBox_input_1.Text) 'max value
        Write_Buf(8) = ThrH '(Convert.ToByte(TextBox_input_1.Text)) \ 256

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&HC9, CRC8_Byte)    'write cmd
#If 1 Then

        For i = 1 To Write_Buf(0)
            CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(i), CRC8_Byte)
        Next
        Write_Buf(i) = CRC8_Byte
#End If

        ' successful, display results      
        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "

        For i = 1 To Write_Buf(0)
            Return_Str = Convert.ToString(Write_Buf(i), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "
        Next
        Return_Str = Convert.ToString(Write_Buf(i), 16).ToUpper

        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Write_Buf_Str & Return_Str

        Dim Data_Len As Byte = 0
#If 1 Then

        If PEC_Sta = True Then
            Data_Len = Write_Buf(0) + 2
        Else
            Data_Len = Write_Buf(0) + 1
        End If
#End If
        If Hardware_Selection = 1 Then

        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, &HC9, Data_Len, Write_Buf, Return_Str)) Then
                    Append_Text1("Calibration Write Block Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Calibration Writing Data to the Device" & vbCrLf)
                    Pic_Kit_Error = True
                    For i = 0 To Write_Buf(0)
                        Write_Buf(i) = 0
                    Next
                End If
            End If
        End If

        Return_Str = "-"
    End Sub
    '#Region
#End Region
#End Region

#Region "Pmbus Controls & Functions"
#Region "Pmbus Controls"


    Private Sub CheckBox2_CheckedChanged(sender As System.Object, e As System.EventArgs)
        If CheckBox2.Checked = True Then
            Mux_Sta = True
            'RadioButton9.Checked = True
            'RadioButton9.BackColor = Color.GreenYellow
        Else
            Mux_Sta = False
            RadioButton9.Checked = False
            RadioButton10.Checked = False
            RadioButton11.Checked = False
            RadioButton9.BackColor = Color.Transparent
            RadioButton10.BackColor = Color.Transparent
            RadioButton11.BackColor = Color.Transparent
        End If
    End Sub
    Private Sub RadioButton9_Click(sender As System.Object, e As System.EventArgs) Handles RadioButton9.Click, RadioButton9.Click, RadioButton9.Click

        If Mux_Sta = True Then
            Thread.Sleep(100)
            Sel_Mux_Ch(1, 0)    'Disable Mux 1 Ports
            Thread.Sleep(200)
            Sel_Mux_Ch(0, 1)

            RadioButton9.Checked = True
            RadioButton9.BackColor = Color.GreenYellow
            RadioButton10.Checked = False
            RadioButton10.BackColor = Color.Transparent
            RadioButton11.Checked = False
            RadioButton11.BackColor = Color.Transparent
        End If


    End Sub
    Private Sub RadioButton10_Click(sender As System.Object, e As System.EventArgs) Handles RadioButton10.Click, RadioButton10.Click, RadioButton10.Click

        If Mux_Sta = True Then
            Thread.Sleep(100)
            Sel_Mux_Ch(1, 0)    'Disable Mux 1 Ports
            Thread.Sleep(200)
            Sel_Mux_Ch(0, 2)
            RadioButton10.Checked = True
            RadioButton10.BackColor = Color.GreenYellow
            RadioButton9.Checked = False
            RadioButton9.BackColor = Color.Transparent
            RadioButton11.Checked = False
            RadioButton11.BackColor = Color.Transparent
        End If

    End Sub
    Private Sub RadioButton11_Click(sender As System.Object, e As System.EventArgs) Handles RadioButton11.Click, RadioButton11.Click, RadioButton11.Click

        If Mux_Sta = True Then
            Thread.Sleep(100)
            Sel_Mux_Ch(1, 0)    'Disable Mux 1 Ports
            Thread.Sleep(200)
            Sel_Mux_Ch(0, 3)
            RadioButton11.Checked = True
            RadioButton11.BackColor = Color.GreenYellow
            RadioButton9.Checked = False
            RadioButton9.BackColor = Color.Transparent
            RadioButton10.Checked = False
            RadioButton10.BackColor = Color.Transparent
        End If

    End Sub

    Private Sub Button5_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        Send_Byte(&H3)
    End Sub

#If 0 Then

    Private Sub Button56_Click(sender As System.Object, e As System.EventArgs)
        ' Page Command
        Write_Buf(0) = Convert.ToByte(NumericUpDown16.Value, 10)
        Page_sel = Convert.ToByte(NumericUpDown16.Value, 10)
        Write_Byte(&H0)
    End Sub


    Private Sub Button50_Click(sender As System.Object, e As System.EventArgs)
        Write_Buf(0) = Convert.ToByte(NumericUpDown12.Value, 10)
        Write_Buf(1) = 0
        Write_Word(&H3B)
    End Sub
    Private Sub Button49_Click(sender As System.Object, e As System.EventArgs)
        'Read Fan Duty
        Dim str As String = Read_Word(&H3B)
        TextBox51.Text = Pmb_Hex_Data
    End Sub



    Private Sub Button26_Click(sender As System.Object, e As System.EventArgs)
        Write_Buf(0) = Convert.ToByte(NumericUpDown2.Value, 10)
        Write_Buf(1) = 0
        Write_Word(&HD0)
    End Sub


    Private Sub Button29_Click(sender As Object, e As EventArgs)
        'Read Vbulk
        Dim str As String = Read_Word(&H8A) 'Read_Word(&HD0)
        TextBox28.Text = Pmb_Hex_Data
    End Sub
#End If
    Private Sub Button52_Click(sender As System.Object, e As System.EventArgs)
        'Enable PMbus WP
        Write_Buf(0) = &H80
        Write_Byte(&H10)
    End Sub
    Private Sub Button51_Click(sender As System.Object, e As System.EventArgs)
        'Disable Pmbus WP
        Write_Buf(0) = &H40
        Write_Byte(&H10)
    End Sub
    Private Sub Button12_Click(sender As System.Object, e As System.EventArgs)
        Write_Buf(0) = &H20
        Write_Byte(&H10)
    End Sub

    Private Sub Button13_Click(sender As System.Object, e As System.EventArgs)
        Write_Buf(0) = &H0
        Write_Byte(&H10)
    End Sub

    Private Sub Button10_Click(sender As System.Object, e As System.EventArgs)
        'Enable EEP Wr
        Write_Buf(0) = &H9A
        Write_Byte(&HEA)
    End Sub

    Private Sub Button9_Click(sender As System.Object, e As System.EventArgs)
        'Disable EEP Wr
        Write_Buf(0) = &H56
        Write_Byte(&HEA)
    End Sub

    Private Sub Button55_Click(sender As System.Object, e As System.EventArgs)
        'Start ORing Test
        Write_Buf(0) = &H1
        Write_Byte(&H74)
    End Sub

    Private Sub Button53_Click(sender As System.Object, e As System.EventArgs)
        'Reset OR-ing Latch
        Write_Buf(0) = &HFA
        Write_Buf(1) = &H0
        Write_Word(&HFB)
    End Sub
#If remove Then
    Private Sub Button23_Click(sender As System.Object, e As System.EventArgs) Handles Button23.Click
        'Dim Val As UInteger = 0
        'Val = Convert.ToUInt16(TextBox16.Text, 10)

        Write_Buf(0) = ON_OFF_Config
        Write_Byte(&H2)
    End Sub
#End If
#If 0 Then

    Private Sub Button24_Click(sender As System.Object, e As System.EventArgs)
        'Read Fan Duty
        Dim str As String = Read_Byte(&H2)
        TextBox17.Text = Pmb_Hex_Data
    End Sub
#End If
    Private Sub Button63_Click(sender As System.Object, e As System.EventArgs)
        'Dim Return_Str As String = ""
        'Dim SMB_Mask As Integer = 0

        'Slave_Addr = NumericUpDown1.Value

        'Write_Buf(0) = 4 ' Count
        'Write_Buf(1) = NumericUpDown21.Value ' Status Vout Command
        'Write_Buf(2) = &H1B ' SMB Alert Command 
        'Write_Buf(3) = &H7A ' Status Vout Command

        ''Compute CRC 8 
        'CRC8_Byte = 0
        'CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        'CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        'CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        'CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        'CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        'CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        'CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        'If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

        '    'Thread.Sleep(1)

        '    Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        '    If Not Return_Str.Length = 2 Then
        '        Return_Str = "0" & Return_Str
        '    End If
        '    Write_Buf_Str = Return_Str & " "
        '    Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
        '    If Not Return_Str.Length = 2 Then
        '        Return_Str = "0" & Return_Str
        '    End If
        '    Write_Buf_Str = Write_Buf_Str & Return_Str & " "

        '    Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

        '    If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

        '        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
        '        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

        '        Read_Buf_Str = " "
        '        Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
        '        If Not Return_Str.Length = 2 Then
        '            Return_Str = "0" & Return_Str
        '        End If
        '        Read_Buf_Str = Read_Buf_Str & Return_Str & " "

        '        Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
        '        If Not Return_Str.Length = 2 Then
        '            Return_Str = "0" & Return_Str
        '        End If
        '        Read_Buf_Str = Read_Buf_Str & Return_Str & " "

        '        If CRC8_Byte = Read_Buf(2) Then
        '            SMB_Mask = Read_Buf(1)
        '            TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
        '            Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
        '        Else
        '            Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
        '        End If

        '    Else
        '        Append_Text1("Error Reading Data From the Device" & vbCrLf)
        '        Pic_Kit_Error = True
        '    End If
        'Else
        '    Append_Text1("Error Writing Data to the Device" & vbCrLf)
        '    Pic_Kit_Error = True
        'End If
    End Sub
    Private Sub Button78_Click(sender As System.Object, e As System.EventArgs)
        Write_Buf(0) = &H1
        Write_Byte(&HE0)
    End Sub
    Private Sub Button18_Click_1(sender As System.Object, e As System.EventArgs)
        Write_Buf(0) = &H0
        Write_Byte(&HE0)
    End Sub

    Private Sub Button17_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        'PSU ON
        Write_Buf(0) = &H80
        Write_Byte(&H1)
    End Sub
    Private Sub Button16_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        'PSU OFF
        Write_Buf(0) = &H0
        Write_Byte(&H1)
    End Sub
#If Remove Then
    Private Sub Button48_Click(sender As System.Object, e As System.EventArgs)
        Append_Text1("Started to Read PMBus Constant Data........" & vbCrLf)
        'Update_Pmbus_Constant(0)
    End Sub
#End If
#If 0 Then

    Private Sub Button19_Click(sender As System.Object, e As System.EventArgs)
        Read_Block(&H86, 6)
        TextBox8.Text = (Read_Buf(1)) + (Read_Buf(2) * 256)
        TextBox11.Text = Read_Buf(3)
        TextBox12.Text = Read_Buf(4) + (Read_Buf(5) * 256) + (Read_Buf(6) * 65536)
    End Sub
    Private Sub Button20_Click(sender As System.Object, e As System.EventArgs)
        Read_Block(&H87, 6)
        TextBox9.Text = (Read_Buf(1)) + (Read_Buf(2) * 256)
        TextBox13.Text = Read_Buf(3)
        TextBox14.Text = Read_Buf(4) + (Read_Buf(5) * 256) + (Read_Buf(6) * 65536)
    End Sub
#End If
    Private Sub Button22_Click(sender As System.Object, e As System.EventArgs)
        'Read_Block(&H30, 5)
        Dim str2 As String = "-"
        'str2 = Convert.ToString(Read_Buf(1), 16) & " "
        'str2 = str2 & Convert.ToString(Read_Buf(2), 16) & " "
        'str2 = str2 & Convert.ToString(Read_Buf(3), 16) & " "
        'str2 = str2 & Convert.ToString(Read_Buf(4), 16) & " "
        'TextBox15.Text = str2 & Convert.ToString(Read_Buf(5), 16)

        Dim Return_Str As String = ""


        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 2 ' Count
        Write_Buf(1) = &H86 'Command Code
        Write_Buf(2) = &H1  'Read Co eff

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H30, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H30, 3, Write_Buf, Return_Str)) Then
            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(2), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H30, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 7, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(2), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(3), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(4), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(5), CRC8_Byte)

                If CRC8_Byte = Read_Buf(6) Then

                    str2 = Convert.ToString(Read_Buf(1), 16) & " "
                    str2 = str2 & Convert.ToString(Read_Buf(2), 16) & " "
                    str2 = str2 & Convert.ToString(Read_Buf(3), 16) & " "
                    str2 = str2 & Convert.ToString(Read_Buf(4), 16) & " "
                    'TextBox15.Text = str2 & Convert.ToString(Read_Buf(5), 16)

                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H30, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H30, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub
#If 0 Then

    Private Sub Button25_Click(sender As System.Object, e As System.EventArgs)
        'Read_Block(&H30, 5)
        Dim str2 As String = "-"
        'str2 = Convert.ToString(Read_Buf(1), 16) & " "
        'str2 = str2 & Convert.ToString(Read_Buf(2), 16) & " "
        'str2 = str2 & Convert.ToString(Read_Buf(3), 16) & " "
        'str2 = str2 & Convert.ToString(Read_Buf(4), 16) & " "
        'TextBox15.Text = str2 & Convert.ToString(Read_Buf(5), 16)

        Dim Return_Str As String = ""

        Slave_Addr = NumericUpDown1.Value

        Write_Buf(0) = 2 ' Count
        Write_Buf(1) = &H87 'Command Code
        Write_Buf(2) = &H1  'Read Coeff

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H30, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H30, 3, Write_Buf, Return_Str)) Then
            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(2), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H30, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 7, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(2), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(3), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(4), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(5), CRC8_Byte)

                If CRC8_Byte = Read_Buf(6) Then

                    str2 = Convert.ToString(Read_Buf(1), 16) & " "
                    str2 = str2 & Convert.ToString(Read_Buf(2), 16) & " "
                    str2 = str2 & Convert.ToString(Read_Buf(3), 16) & " "
                    str2 = str2 & Convert.ToString(Read_Buf(4), 16) & " "
                    TextBox18.Text = str2 & Convert.ToString(Read_Buf(5), 16)

                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H30, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H30, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If
    End Sub
#End If
    Private Sub CheckBox16_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox16.CheckedChanged
        Update_Vout(0)
    End Sub
    Private Sub CheckBox19_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox19.CheckedChanged
        Update_Vout(0)
    End Sub
    Private Sub CheckBox27_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox27.CheckedChanged
        Update_Iout(0)
    End Sub
    Private Sub CheckBox25_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox25.CheckedChanged
        Update_Iout(0)
    End Sub
    Private Sub CheckBox43_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox43.CheckedChanged
        Update_Input(0)
    End Sub
    Private Sub CheckBox42_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox42.CheckedChanged
        Update_Input(0)
    End Sub
    Private Sub CheckBox41_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox41.CheckedChanged
        Update_Input(0)
    End Sub
    Private Sub CheckBox40_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox40.CheckedChanged
        Update_Input(0)
    End Sub
    Private Sub CheckBox39_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox39.CheckedChanged
        Update_Input(0)
    End Sub
    Private Sub CheckBox37_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox37.CheckedChanged
        Update_Input(0)
    End Sub
    Private Sub CheckBox44_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox44.CheckedChanged
        Update_Input(0)
    End Sub
    Private Sub CheckBox34_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox34.CheckedChanged
        Update_Temp(0)
    End Sub
    Private Sub CheckBox35_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox35.CheckedChanged
        Update_Temp(0)
    End Sub
    Private Sub CheckBox51_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox51.CheckedChanged
        Update_Fan(0)
    End Sub
    Private Sub CheckBox49_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox49.CheckedChanged
        Update_Fan(0)
    End Sub
    Private Sub CheckBox47_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox47.CheckedChanged
        Update_Fan(0)
    End Sub
    Private Sub CheckBox59_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox59.CheckedChanged
        Update_CML(0)
    End Sub
    Private Sub CheckBox58_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox58.CheckedChanged
        Update_CML(0)
    End Sub
    Private Sub CheckBox57_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox57.CheckedChanged
        Update_CML(0)
    End Sub
    Private Sub CheckBox56_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox56.CheckedChanged
        Update_CML(0)
    End Sub
    Private Sub CheckBox55_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox55.CheckedChanged
        Update_CML(0)
    End Sub
    Private Sub CheckBox54_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox54.CheckedChanged
        Update_CML(0)
    End Sub
    Private Sub CheckBox53_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox53.CheckedChanged
        Update_CML(0)
    End Sub
    Private Sub CheckBox60_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox60.CheckedChanged
        Update_CML(0)
    End Sub
    Private Sub CheckBox68_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox68.CheckedChanged
        Update_MFR(0)
    End Sub
    Private Sub CheckBox67_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox67.CheckedChanged
        Update_MFR(0)
    End Sub
    Private Sub CheckBox64_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox64.CheckedChanged
        Update_MFR(0)
    End Sub
    Private Sub CheckBox12_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox12.CheckedChanged
        Update_OTHER(0)
    End Sub
    Private Sub CheckBox10_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox10.CheckedChanged
        Update_OTHER(0)
    End Sub
    Private Sub CheckBox6_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox6.CheckedChanged
        Update_OTHER(0)
    End Sub
    Private Sub CheckBox69_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox69.CheckedChanged
        Update_OTHER(0)
    End Sub
    Private Sub CheckBox4_CheckedChanged(sender As System.Object, e As System.EventArgs)
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub CheckBox72_CheckedChanged(sender As System.Object, e As System.EventArgs)
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub CheckBox73_CheckedChanged(sender As System.Object, e As System.EventArgs)
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub CheckBox74_CheckedChanged(sender As System.Object, e As System.EventArgs)
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub CheckBox3_CheckedChanged(sender As System.Object, e As System.EventArgs)
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub Update_ON_OFF_Config(ByVal Word_Addr As Byte)
        'Dim ON_OFF_Config As Byte = 0
#If remove Then
        If CheckBox4.Checked = True Then
            ON_OFF_Config = &H10
        End If

        If CheckBox72.Checked = True Then
            ON_OFF_Config = ON_OFF_Config + &H8
        End If
        If CheckBox73.Checked = True Then
            ON_OFF_Config = ON_OFF_Config + &H4
        End If
        If CheckBox74.Checked = True Then
            ON_OFF_Config = ON_OFF_Config + &H2
        End If
        If CheckBox3.Checked = True Then
            ON_OFF_Config = ON_OFF_Config + &H1
        End If

        If ON_OFF_Config = &H11 Then
            Label69.Text = "Control Pin & Operation Command Disbaled"
            'Both Control Pin & Operation Command Disbaled
        ElseIf ON_OFF_Config = &H1D Then
            Label69.Text = "Control Pin & Operation Command Enabled" 'Both Control Pin & Operation Command Enabled
        ElseIf ON_OFF_Config = &H15 Then
            Label69.Text = "Disabled Operation & Enable Control Pin" 'Disabled Operation & Enable Control Pin
        ElseIf ON_OFF_Config = &H19 Then
            Label69.Text = "Enable Operation Command and Control Pin Disabled" 'Enable Operation Command and Control Pin Disabled
        End If
        Dim str As String = Convert.ToString(ON_OFF_Config, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox16.Text = str
#End If
    End Sub
    Private Sub Update_Vout(ByVal Word_Addr As Byte)
        Dim Vout_Mask As Byte = 0
        If CheckBox19.Checked = True Then
            Vout_Mask = &H80
        End If
        If CheckBox16.Checked = True Then
            Vout_Mask = Vout_Mask + &H10
        End If

        Dim str As String = Convert.ToString(Vout_Mask, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox27.Text = str
    End Sub
    Private Sub Update_Iout(ByVal Word_Addr As Byte)
        Dim Iout_Mask As Byte = 0
        If CheckBox27.Checked = True Then
            Iout_Mask = &H80
        End If
        If CheckBox25.Checked = True Then
            Iout_Mask = Iout_Mask + &H20
        End If

        Dim str As String = Convert.ToString(Iout_Mask, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox23.Text = str
    End Sub
    Private Sub Update_Temp(ByVal Word_Addr As Byte)
        Dim Mask As Byte = 0
        If CheckBox35.Checked = True Then
            Mask = &H80
        End If
        If CheckBox34.Checked = True Then
            Mask = Mask + &H40
        End If

        Dim str As String = Convert.ToString(Mask, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox21.Text = str
    End Sub
    Private Sub Update_Input(ByVal Word_Addr As Byte)
        Dim Mask As Byte = 0

        If CheckBox43.Checked = True Then
            Mask = &H80
        End If
        If CheckBox42.Checked = True Then
            Mask = Mask + &H40
        End If
        If CheckBox40.Checked = True Then
            Mask = Mask + &H10
        End If

        Dim str As String = Convert.ToString(Mask, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox7.Text = str
    End Sub
    Private Sub Update_Fan(ByVal Word_Addr As Byte)
        Dim Mask As Byte = 0
        If CheckBox51.Checked = True Then
            Mask = &H80
        End If
        'If CheckBox49.Checked = True Then
        '    Mask = Mask + &H20
        'End If
        If CheckBox47.Checked = True Then
            Mask = Mask + &H8
        End If

        Dim str As String = Convert.ToString(Mask, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox5.Text = str
    End Sub
    Private Sub Update_CML(ByVal Word_Addr As Byte)
        Dim Mask As Byte = 0
        If CheckBox59.Checked = True Then
            Mask = &H80
        End If
        If CheckBox58.Checked = True Then
            Mask = Mask + &H40
        End If
        If CheckBox57.Checked = True Then
            Mask = Mask + &H20
        End If
        If CheckBox55.Checked = True Then
            Mask = Mask + &H8
        End If
        If CheckBox53.Checked = True Then
            Mask = Mask + &H2
        End If
        If CheckBox60.Checked = True Then
            Mask = Mask + &H1
        End If
        Dim str As String = Convert.ToString(Mask, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox3.Text = str
    End Sub
    Private Sub Update_MFR(ByVal Word_Addr As Byte)
        Dim Mfr_Mask As Byte = 0
        If CheckBox68.Checked = True Then
            Mfr_Mask = &H1
        End If
        'If CheckBox64.Checked = True Then
        '    Vsb_Mask = Vsb_Mask + &H10
        'End If

        Dim str As String = Convert.ToString(Mfr_Mask, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox1.Text = str
    End Sub
    Private Sub Update_OTHER(ByVal Word_Addr As Byte)
        Dim Other_Mask As Byte = 0
        If CheckBox6.Checked = True Then
            Other_Mask = &H2
        End If
        If CheckBox69.Checked = True Then
            Other_Mask = Other_Mask + &H1
        End If

        Dim str As String = Convert.ToString(Other_Mask, 16).ToUpper
        If str.Length = 1 Then
            str = "0" & str
        End If
        TextBox31.Text = str
    End Sub


#If 0 Then

    Private Sub Button7_Click(sender As System.Object, e As System.EventArgs) Handles Button_Status_VOUT.Click

        Dim Data0 As Byte = Convert.ToByte(TextBox27.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H7A      'Status Vout Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)

    End Sub
#End If
    Private Sub Button_Status_VOUT_Click(sender As System.Object, e As System.EventArgs) Handles Button_Status_VOUT.Click

        Dim Data0 As Byte = Convert.ToByte(TextBox27.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H7A      'Status Vout Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)

    End Sub

    Private Sub Button4_Click(sender As System.Object, e As System.EventArgs) Handles Button4.Click

        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 3 ' Count
        Write_Buf(1) = NumericUpDown21.Value ' Status Vout Command
        Write_Buf(2) = &H1B ' SMB Alert Command 
        Write_Buf(3) = &H7A ' Status Vout Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)
            'Thread.Sleep(1)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub

    Private Sub Button90_Click(sender As System.Object, e As System.EventArgs) Handles Button90.Click

        Dim Data0 As Byte = Convert.ToByte(TextBox23.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H7B      'Status Iout Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)
    End Sub

    Private Sub Button89_Click(sender As System.Object, e As System.EventArgs) Handles Button89.Click

        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown_S_Addr.Value ' Status Vout Command
        Write_Buf(2) = &H1B ' SMB Alert Command 
        Write_Buf(3) = &H7B ' Status Iout Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub

    Private Sub Button92_Click(sender As System.Object, e As System.EventArgs) Handles Button92.Click
        Dim Data0 As Byte = Convert.ToByte(TextBox21.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H7D      'Status Temp Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)
    End Sub

    Private Sub Button91_Click(sender As System.Object, e As System.EventArgs) Handles Button91.Click
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value ' Status Vout Command
        Write_Buf(2) = &H1B ' SMB Alert Command 
        Write_Buf(3) = &H7D ' Status Temp Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub

    Private Sub Button96_Click(sender As System.Object, e As System.EventArgs) Handles Button96.Click

        Dim Data0 As Byte = Convert.ToByte(TextBox7.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H7C      'Status Input Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)
    End Sub

    Private Sub Button95_Click(sender As System.Object, e As System.EventArgs) Handles Button95.Click
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value ' Status Vout Command
        Write_Buf(2) = &H1B ' SMB Alert Command 
        Write_Buf(3) = &H7C ' Status input Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub

    Private Sub Button98_Click(sender As System.Object, e As System.EventArgs) Handles Button98.Click

        Dim Data0 As Byte = Convert.ToByte(TextBox5.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H81      'Status Fan Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)
    End Sub

    Private Sub Button97_Click(sender As System.Object, e As System.EventArgs) Handles Button97.Click
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value ' page
        Write_Buf(2) = &H1B ' SMB Alert Command 
        Write_Buf(3) = &H81 ' Status Fan Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub

    Private Sub Button100_Click(sender As System.Object, e As System.EventArgs) Handles Button100.Click

        Dim Data0 As Byte = Convert.ToByte(TextBox3.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H7E      'Status CML Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)
    End Sub

    Private Sub Button99_Click(sender As System.Object, e As System.EventArgs) Handles Button99.Click
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value ' Status Vout Command
        Write_Buf(2) = &H1B ' SMB Alert Command 
        Write_Buf(3) = &H7E ' Status CML Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub

    Private Sub Button102_Click(sender As System.Object, e As System.EventArgs) Handles Button102.Click

        Dim Data0 As Byte = Convert.ToByte(TextBox1.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H80      'Status mfr Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)
    End Sub

    Private Sub Button101_Click(sender As System.Object, e As System.EventArgs) Handles Button101.Click
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value ' page
        Write_Buf(2) = &H1B ' SMB Alert Command 
        Write_Buf(3) = &H80 ' Status mfr Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub

    Private Sub Button104_Click(sender As System.Object, e As System.EventArgs) Handles Button104.Click

        Dim Data0 As Byte = Convert.ToByte(TextBox1.Text Mod 256, 16)

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value 'Page
        Write_Buf(2) = &H1B ''SMB Alert Mask Command
        Write_Buf(3) = &H7F      'Status Other Command
        Write_Buf(4) = Data0 'Mask Value
        Write_Block(&H5)
    End Sub

    Private Sub Button103_Click(sender As System.Object, e As System.EventArgs) Handles Button103.Click
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown21.Value ' page
        Write_Buf(2) = &H1B ' SMB Alert Command 
        Write_Buf(3) = &H7F ' Status other Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "
            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox89.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If

    End Sub
#If remove Then
    Private Sub Button105_Click(sender As System.Object, e As System.EventArgs)

        Dim Addr_cmd_code As Byte = Convert.ToByte(TextBox10.Text, 16)
        Dim Data0 As Byte = Convert.ToByte(TextBox19.Text Mod 256, 16)
        Dim Data1 As Byte = Convert.ToByte(TextBox19.Text \ 256, 16)

        Write_Buf(0) = Convert.ToByte(TextBox32.Text, 10) ' Count
        Write_Buf(1) = Convert.ToByte(NumericUpDown15.Value, 10) 'Page
        Write_Buf(2) = Addr_cmd_code      'Command Code
        Write_Buf(3) = Data0 'Data LSB
        Write_Buf(4) = Data1 'Data MSB

        Write_Block(&H5)
    End Sub
#End If
    Private Sub Button106_Click(sender As System.Object, e As System.EventArgs)
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown_S_Addr.Value
#If Remove Then
        Write_Buf(0) = Convert.ToByte(TextBox69.Text, 10) ' Count
        Write_Buf(1) = Convert.ToByte(NumericUpDown17.Value, 10) 'Page
        Write_Buf(2) = Convert.ToByte(TextBox68.Text, 10)  'Command Code
#End If
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H6, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 3, Write_Buf, Return_Str)) Then
            'Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "

            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Return_Str = Convert.ToString(Write_Buf(2), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 4, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)
                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper

                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(3) Then

                    'TextBox75.Text = Convert.ToString(Read_Buf(0), 10).ToUpper

                    If Read_Buf(0) > 1 Then
                        SMB_Mask = Read_Buf(1) + (Read_Buf(2) * 256)
                    Else
                        SMB_Mask = Read_Buf(1)
                    End If

                    '       TextBox71.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If
    End Sub

    Private Sub Button15_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        If Capture_Data = False Then
            SaveFileDialog1.FileName = "Fan_Profile_xx'C_xL_xxx.csv"
            SaveFileDialog1.InitialDirectory = "C:\Users\Meravanagikiran\Documents"

            If SaveFileDialog1.ShowDialog() = System.Windows.Forms.DialogResult.OK Then
                Dim bytes() As Byte
                bytes = Encoding.ASCII.GetBytes("Date & Time,T Ambient,T Hotspot,T Outlet,Fan Speed" & vbCrLf)

                Log_File_Name = SaveFileDialog1.FileName
                My.Computer.FileSystem.WriteAllBytes(Log_File_Name, bytes, False)
                '             Button15.BackColor = Color.Green
                Data_Arr_Pntr = 0
                Capture_Data = True
                Capture_Data_Pntr = 0
            End If
        End If
    End Sub
    Private Sub Button6_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        If Capture_Data = True Then
            Capture_Data = False
            '       'Button6.BackColor = Color.Yellow
            '  ElseIf Button15.BackColor = Color.Green Then
            '      Capture_Data = True
            '      Button6.BackColor = Color.Transparent
        End If
    End Sub

#If remove Then
    Private Sub Button11_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        If Capture_Data = True Then
            Capture_Data = False
            Update_Log_to_file(0)
            'Button15.BackColor = Color.Transparent
        End If
    End Sub
#End If
#End Region


#Region "Pmbus Functions"
    Private Sub Init_Pmbus_DGV(ByVal Null_Data As Byte)
        Dim Arr_Len As Byte = PMBus_Data_Struct.Length
        For Arr_Loc As Byte = 0 To Arr_Len - 1
            DataGridView1.Rows.Add(1)
            DataGridView1.Rows(Arr_Loc).Cells(0).Value() = Convert.ToString(PMBus_Data_Struct(Arr_Loc).Command, 16).ToUpper
            DataGridView1.Rows(Arr_Loc).Cells(1).Value() = PMBus_Data_Struct(Arr_Loc).Cmd_Name
        Next
    End Sub

#If remove Then
    Private Sub Init_Pmbus_Cnst_DGV(ByVal Null_Data As Byte)
        Dim Arr_Len As Byte = PMBus_Cnst_Struct.Length
        For Arr_Loc As Byte = 0 To Arr_Len - 1
            DataGridView4.Rows.Add(1)
            DataGridView4.Rows(Arr_Loc).Cells(0).Value() = Convert.ToString(PMBus_Cnst_Struct(Arr_Loc).Command, 16).ToUpper
            DataGridView4.Rows(Arr_Loc).Cells(1).Value() = PMBus_Cnst_Struct(Arr_Loc).Cmd_Name
        Next
    End Sub
#End If

    Private Sub Init_Pmbus_Cnst_1_DGV(ByVal Null_Data As Byte)
        Dim Arr_Len As Byte = PMBus_Cnst_Struct.Length
        For Arr_Loc As Byte = 0 To Arr_Len - 1
            DataGridView2.Rows.Add(1)
            DataGridView2.Rows(Arr_Loc).Cells(0).Value() = Convert.ToString(PMBus_Cnst_Struct(Arr_Loc).Command, 16).ToUpper
            DataGridView2.Rows(Arr_Loc).Cells(1).Value() = PMBus_Cnst_Struct(Arr_Loc).Cmd_Name
        Next
    End Sub

    Private Sub Init_Pmbus_MFR_DGV(ByVal Null_Data As Byte)
        Dim Arr_Len As Byte = PMBus_MFR_Struct.Length
        For Arr_Loc As Byte = 0 To Arr_Len - 1
            DataGridView_MFR.Rows.Add(1)
            DataGridView_MFR.Rows(Arr_Loc).Cells(0).Value() = Convert.ToString(PMBus_MFR_Struct(Arr_Loc).Command, 16).ToUpper
            DataGridView_MFR.Rows(Arr_Loc).Cells(1).Value() = PMBus_MFR_Struct(Arr_Loc).Cmd_Name
        Next
    End Sub

#End Region
#Region "cips structure"
    Private Sub Init_Pmbus_Log_DGV(ByVal Null_Data As Byte)
        Dim Arr_Len As Byte = PMBus_LOG_Struct.Length
        For Arr_Loc As Byte = 0 To Arr_Len - 1
            DataGridView_LOG.Rows.Add(1)
            DataGridView_LOG.Rows(Arr_Loc).Cells(0).Value() = Convert.ToString(PMBus_LOG_Struct(Arr_Loc).Command, 16).ToUpper
            DataGridView_LOG.Rows(Arr_Loc).Cells(1).Value() = PMBus_LOG_Struct(Arr_Loc).Cmd_Name
        Next
    End Sub

#If remove Then
    Private Sub Update_Pmbus_Constant(ByVal Null_Data As Byte)
        Dim str As String
        Dim Arr_Len As Byte = PMBus_Cnst_Struct.Length
        For Arr_Loc = 0 To Arr_Len - 1
            If PMBus_Cnst_Struct(Arr_Loc).RW_Length = 1 Then
                If Page_sel = 1 Then

                    If PMBus_Cnst_Struct(Arr_Loc).Command = &H20 Then

                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_Cnst_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else

                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = "-"

                    End If

                Else

                    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_Cnst_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                End If


            ElseIf PMBus_Cnst_Struct(Arr_Loc).RW_Length = 2 Then

                If Page_sel = 1 Then

                    If PMBus_Cnst_Struct(Arr_Loc).Command = &H40 Or PMBus_Cnst_Struct(Arr_Loc).Command = &H46 Or PMBus_Cnst_Struct(Arr_Loc).Command = &H4A Then

                        If PMBus_Cnst_Struct(Arr_Loc).Data = False Then
                            DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_Cnst_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                            DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Cnst(PMBus_Cnst_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        End If
                    Else
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = "-"
                    End If

                Else

                    If PMBus_Cnst_Struct(Arr_Loc).Data = False Then
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_Cnst_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        If Not PMBus_Cnst_Struct(Arr_Loc).Command = &H4A Then
                            DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Cnst(PMBus_Cnst_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView4.Rows(Arr_Loc).Cells(2).Value() = "-"
                            DataGridView4.Rows(Arr_Loc).Cells(3).Value() = "-"
                            DataGridView4.Rows(Arr_Loc).Cells(4).Value() = "-"
                        End If


                    End If
                End If

            ElseIf PMBus_Cnst_Struct(Arr_Loc).RW_Length > 2 Then

                If Page_sel = 1 Then

                    'If PMBus_Data_Struct(Arr_Loc).Command = &H30 Then

                    '    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_Cnst_Struct(Arr_Loc).Command, PMBus_Cnst_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                    '    str = Convert.ToString(Read_Buf(1), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(2), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(3), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(4), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(5), 16) & " "
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str & Convert.ToString(Read_Buf(6), 16)
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = ascii.GetString(Read_Buf, 1, 6)

                    'Else
                    '    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = "-"
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = "-"
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = "-"
                    'End If
                Else

                    If PMBus_Cnst_Struct(Arr_Loc).Command = &HF5 Or PMBus_Cnst_Struct(Arr_Loc).Command = &HF6 Then

                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Block_2(PMBus_Cnst_Struct(Arr_Loc).Command, PMBus_Cnst_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_Cnst_Struct(Arr_Loc).Command, PMBus_Cnst_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    End If


                    If PMBus_Cnst_Struct(Arr_Loc).Command = &H30 Then 'Coefficients

                        'str = Convert.ToString(Read_Buf(1), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        'DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Convert.ToString(Read_Buf(0), 16) & str & Convert.ToString(Read_Buf(6), 16)
                        'DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        'DataGridView4.Rows(Arr_Loc).Cells(4).Value() = ascii.GetString(Read_Buf, 1, 5)

                    ElseIf PMBus_Cnst_Struct(Arr_Loc).Command = &HF5 Then 'Cisco_FW_REVISION

                        str = Convert.ToString(Read_Buf(0), 16) & " "
                        str = str & Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = str '& " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Convert.ToString(Read_Buf(0), 16) & "." & Convert.ToString(Read_Buf(1), 16) & "." & Convert.ToString(Read_Buf(2), 16) & " " & "-" & " " & Convert.ToString(Read_Buf(3), 16) & "." & Convert.ToString(Read_Buf(4), 16) & "." & Convert.ToString(Read_Buf(5), 16)

                    ElseIf PMBus_Cnst_Struct(Arr_Loc).Command = &HF6 Then 'BL_REVISION
                        str = Convert.ToString(Read_Buf(0), 16) & " "
                        str = str & Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = str ' & " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Convert.ToString(Read_Buf(0), 16) & "." & Convert.ToString(Read_Buf(1), 16) & "." & Convert.ToString(Read_Buf(2), 16) & " " & "-" & " " & Convert.ToString(Read_Buf(3), 16) & "." & Convert.ToString(Read_Buf(4), 16) & "." & Convert.ToString(Read_Buf(5), 16)

                    ElseIf PMBus_Cnst_Struct(Arr_Loc).Command = &HF7 Then 'QCS_FW_REVISION
                        str = Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Convert.ToString(Read_Buf(0), 16) & " " & str & " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = str 'ascii.GetString(Read_Buf, 1, 7)
                    End If

                End If

            End If
        Next
    End Sub
#End If

    Private Sub Update_Pmbus_Constant_1(ByVal Null_Data As Byte)
        Dim str As String
        Dim Arr_Len As Byte = PMBus_Cnst_Struct.Length
        For Arr_Loc = 0 To Arr_Len - 1
            If PMBus_Cnst_Struct(Arr_Loc).RW_Length = 1 Then
                If Page_sel = 1 Then

                    If PMBus_Cnst_Struct(Arr_Loc).Command = &H20 Then

                        DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_Cnst_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView2.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else

                        DataGridView2.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView2.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView2.Rows(Arr_Loc).Cells(4).Value() = "-"

                    End If

                Else

                    DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_Cnst_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                    DataGridView2.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                End If


            ElseIf PMBus_Cnst_Struct(Arr_Loc).RW_Length = 2 Then

                If Page_sel = 1 Then

                    If PMBus_Cnst_Struct(Arr_Loc).Command = &H40 Or PMBus_Cnst_Struct(Arr_Loc).Command = &H46 Or PMBus_Cnst_Struct(Arr_Loc).Command = &H4A Then

                        If PMBus_Cnst_Struct(Arr_Loc).Data = False Then
                            DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_Cnst_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                            DataGridView2.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Cnst(PMBus_Cnst_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView2.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        End If
                    Else
                        DataGridView2.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView2.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView2.Rows(Arr_Loc).Cells(4).Value() = "-"
                    End If

                Else

                    If PMBus_Cnst_Struct(Arr_Loc).Data = False Then
                        DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_Cnst_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView2.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        If Not PMBus_Cnst_Struct(Arr_Loc).Command = &H4A Then
                            DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Cnst(PMBus_Cnst_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView2.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView2.Rows(Arr_Loc).Cells(2).Value() = "-"
                            DataGridView2.Rows(Arr_Loc).Cells(3).Value() = "-"
                            DataGridView2.Rows(Arr_Loc).Cells(4).Value() = "-"
                        End If


                    End If
                End If

            ElseIf PMBus_Cnst_Struct(Arr_Loc).RW_Length > 2 Then

                If Page_sel = 1 Then

                    'If PMBus_Data_Struct(Arr_Loc).Command = &H30 Then

                    '    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_Cnst_Struct(Arr_Loc).Command, PMBus_Cnst_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                    '    str = Convert.ToString(Read_Buf(1), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(2), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(3), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(4), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(5), 16) & " "
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str & Convert.ToString(Read_Buf(6), 16)
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = ascii.GetString(Read_Buf, 1, 6)

                    'Else
                    '    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = "-"
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = "-"
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = "-"
                    'End If
                Else

                    If PMBus_Cnst_Struct(Arr_Loc).Command = &HF5 Or PMBus_Cnst_Struct(Arr_Loc).Command = &HF6 Then

                        DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Read_Block_2(PMBus_Cnst_Struct(Arr_Loc).Command, PMBus_Cnst_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView2.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_Cnst_Struct(Arr_Loc).Command, PMBus_Cnst_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView2.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    End If


                    If PMBus_Cnst_Struct(Arr_Loc).Command = &H30 Then 'Coefficients

                        'str = Convert.ToString(Read_Buf(1), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        'DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Convert.ToString(Read_Buf(0), 16) & str & Convert.ToString(Read_Buf(6), 16)
                        'DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        'DataGridView4.Rows(Arr_Loc).Cells(4).Value() = ascii.GetString(Read_Buf, 1, 5)

                    ElseIf PMBus_Cnst_Struct(Arr_Loc).Command = &HF5 Then 'Cisco_FW_REVISION

                        str = Convert.ToString(Read_Buf(0), 16) & " "
                        str = str & Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
#If remove Then
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = str '& " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Convert.ToString(Read_Buf(0), 16) & "." & Convert.ToString(Read_Buf(1), 16) & "." & Convert.ToString(Read_Buf(2), 16) & " " & "-" & " " & Convert.ToString(Read_Buf(3), 16) & "." & Convert.ToString(Read_Buf(4), 16) & "." & Convert.ToString(Read_Buf(5), 16)
#End If
                    ElseIf PMBus_Cnst_Struct(Arr_Loc).Command = &HF6 Then 'BL_REVISION
                        str = Convert.ToString(Read_Buf(0), 16) & " "
                        str = str & Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView2.Rows(Arr_Loc).Cells(2).Value() = str ' & " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView2.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView2.Rows(Arr_Loc).Cells(4).Value() = Convert.ToString(Read_Buf(0), 16) & "." & Convert.ToString(Read_Buf(1), 16) & "." & Convert.ToString(Read_Buf(2), 16) & " " & "-" & " " & Convert.ToString(Read_Buf(3), 16) & "." & Convert.ToString(Read_Buf(4), 16) & "." & Convert.ToString(Read_Buf(5), 16)

                    ElseIf PMBus_Cnst_Struct(Arr_Loc).Command = &HF7 Then 'QCS_FW_REVISION
                        str = Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView2.Rows(Arr_Loc).Cells(2).Value() = Convert.ToString(Read_Buf(0), 16) & " " & str & " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView2.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView2.Rows(Arr_Loc).Cells(4).Value() = str 'ascii.GetString(Read_Buf, 1, 7)
                    End If

                End If

            End If
        Next
    End Sub
    Private Sub Update_Pmbus_MFR(ByVal Null_Data As Byte)
        Dim str As String
        Dim Arr_Len As Byte = PMBus_MFR_Struct.Length
        For Arr_Loc = 0 To Arr_Len - 1
            If PMBus_MFR_Struct(Arr_Loc).RW_Length = 1 Then
                If Page_sel = 1 Then

                    If PMBus_MFR_Struct(Arr_Loc).Command = &HA0 Then

                        DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_MFR_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        '' DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else

                        DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = "-"

                    End If

                Else

                    DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_MFR_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                    DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                End If


            ElseIf PMBus_MFR_Struct(Arr_Loc).RW_Length = 2 Then

                If Page_sel = 1 Then

                    If PMBus_MFR_Struct(Arr_Loc).Command = &HA1 Or PMBus_MFR_Struct(Arr_Loc).Command = &HA2 Or PMBus_MFR_Struct(Arr_Loc).Command = &HA3 Then

                        If PMBus_MFR_Struct(Arr_Loc).Data = False Then
                            DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_MFR_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                            DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Cnst(PMBus_MFR_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        End If
                    Else
                        DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = "-"
                    End If

                Else

                    If PMBus_MFR_Struct(Arr_Loc).Data = False Then
                        DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_MFR_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        If Not PMBus_MFR_Struct(Arr_Loc).Command = &H4A Then
                            DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Cnst(PMBus_MFR_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = "-"
                            DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = "-"
                            DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = "-"
                        End If


                    End If
                End If

            ElseIf PMBus_MFR_Struct(Arr_Loc).RW_Length > 2 Then

                If Page_sel = 1 Then

                    'If PMBus_Data_Struct(Arr_Loc).Command = &H30 Then

                    '    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_Cnst_Struct(Arr_Loc).Command, PMBus_Cnst_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                    '    str = Convert.ToString(Read_Buf(1), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(2), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(3), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(4), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(5), 16) & " "
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str & Convert.ToString(Read_Buf(6), 16)
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = ascii.GetString(Read_Buf, 1, 6)

                    'Else
                    '    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = "-"
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = "-"
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = "-"
                    'End If
                Else

                    If PMBus_MFR_Struct(Arr_Loc).Command = &HF5 Or PMBus_MFR_Struct(Arr_Loc).Command = &HF6 Then

                        DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Read_Block_2(PMBus_MFR_Struct(Arr_Loc).Command, PMBus_MFR_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_MFR_Struct(Arr_Loc).Command, PMBus_MFR_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    End If


                    If PMBus_MFR_Struct(Arr_Loc).Command = &H30 Then 'Coefficients

                        'str = Convert.ToString(Read_Buf(1), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        'DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Convert.ToString(Read_Buf(0), 16) & str & Convert.ToString(Read_Buf(6), 16)
                        'DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        'DataGridView4.Rows(Arr_Loc).Cells(4).Value() = ascii.GetString(Read_Buf, 1, 5)

                    ElseIf PMBus_MFR_Struct(Arr_Loc).Command = &HF5 Then 'Cisco_FW_REVISION

                        str = Convert.ToString(Read_Buf(0), 16) & " "
                        str = str & Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        '   DataGridView4.Rows(Arr_Loc).Cells(2).Value() = str '& " " & Convert.ToString(Read_Buf(7), 16)
                        ' DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        '  DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Convert.ToString(Read_Buf(0), 16) & "." & Convert.ToString(Read_Buf(1), 16) & "." & Convert.ToString(Read_Buf(2), 16) & " " & "-" & " " & Convert.ToString(Read_Buf(3), 16) & "." & Convert.ToString(Read_Buf(4), 16) & "." & Convert.ToString(Read_Buf(5), 16)

                    ElseIf PMBus_MFR_Struct(Arr_Loc).Command = &HF6 Then 'BL_REVISION
                        str = Convert.ToString(Read_Buf(0), 16) & " "
                        str = str & Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        ' DataGridView4.Rows(Arr_Loc).Cells(2).Value() = str ' & " " & Convert.ToString(Read_Buf(7), 16)
                        ' DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        ' DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Convert.ToString(Read_Buf(0), 16) & "." & Convert.ToString(Read_Buf(1), 16) & "." & Convert.ToString(Read_Buf(2), 16) & " " & "-" & " " & Convert.ToString(Read_Buf(3), 16) & "." & Convert.ToString(Read_Buf(4), 16) & "." & Convert.ToString(Read_Buf(5), 16)

                    ElseIf PMBus_MFR_Struct(Arr_Loc).Command = &HF7 Then 'QCS_FW_REVISION
                        str = Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Convert.ToString(Read_Buf(0), 16) & " " & str & " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = str 'ascii.GetString(Read_Buf, 1, 7)
                    End If

                End If

            End If
        Next
    End Sub

    Private Sub Update_Pmbus_Log(ByVal Null_Data As Byte)
        'Dim str As String
        Dim Arr_Len As Byte = PMBus_LOG_Struct.Length
        For Arr_Loc = 0 To Arr_Len - 1
            If PMBus_LOG_Struct(Arr_Loc).RW_Length = 1 Then
                If Page_sel = 1 Then

                    If PMBus_LOG_Struct(Arr_Loc).Command = &HA0 Then

                        DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_LOG_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else

                        DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = "-"

                    End If

                Else

                    DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_LOG_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                    DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                End If


            ElseIf PMBus_LOG_Struct(Arr_Loc).RW_Length = 2 Then

                If Page_sel = 1 Then

                    If PMBus_LOG_Struct(Arr_Loc).Command = &HA1 Or PMBus_LOG_Struct(Arr_Loc).Command = &HA2 Or PMBus_LOG_Struct(Arr_Loc).Command = &HA3 Then

                        If PMBus_LOG_Struct(Arr_Loc).Data = False Then
                            DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_LOG_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                            DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Cnst(PMBus_LOG_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        End If
                    Else
                        DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = "-"
                    End If

                Else

                    If PMBus_LOG_Struct(Arr_Loc).Data = False Then
                        DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_LOG_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        If Not PMBus_LOG_Struct(Arr_Loc).Command = &H4A Then
                            DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Cnst(PMBus_LOG_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = "-"
                            DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = "-"
                            DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = "-"
                        End If


                    End If
                End If

            ElseIf PMBus_LOG_Struct(Arr_Loc).RW_Length > 2 Then

                If Page_sel = 1 Then

                    'If PMBus_Data_Struct(Arr_Loc).Command = &H30 Then

                    '    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_Cnst_Struct(Arr_Loc).Command, PMBus_Cnst_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                    '    str = Convert.ToString(Read_Buf(1), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(2), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(3), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(4), 16) & " "
                    '    str = str & Convert.ToString(Read_Buf(5), 16) & " "
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str & Convert.ToString(Read_Buf(6), 16)
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = ascii.GetString(Read_Buf, 1, 6)

                    'Else
                    '    DataGridView4.Rows(Arr_Loc).Cells(2).Value() = "-"
                    '    DataGridView4.Rows(Arr_Loc).Cells(3).Value() = "-"
                    '    DataGridView4.Rows(Arr_Loc).Cells(4).Value() = "-"
                    'End If
                Else

                    If PMBus_LOG_Struct(Arr_Loc).Command = &HF5 Or PMBus_LOG_Struct(Arr_Loc).Command = &HF6 Then

                        DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = Read_Block_2(PMBus_LOG_Struct(Arr_Loc).Command, PMBus_LOG_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        DataGridView_LOG.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_LOG_Struct(Arr_Loc).Command, PMBus_LOG_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView_LOG.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView_LOG.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    End If

#If 0 Then

                    If PMBus_MFR_Struct(Arr_Loc).Command = &H30 Then 'Coefficients

                        'str = Convert.ToString(Read_Buf(1), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        'str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        'DataGridView4.Rows(Arr_Loc).Cells(2).Value() = Convert.ToString(Read_Buf(0), 16) & str & Convert.ToString(Read_Buf(6), 16)
                        'DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        'DataGridView4.Rows(Arr_Loc).Cells(4).Value() = ascii.GetString(Read_Buf, 1, 5)

                    ElseIf PMBus_LOG_Struct(Arr_Loc).Command = &HF5 Then 'Cisco_FW_REVISION

                        str = Convert.ToString(Read_Buf(0), 16) & " "
                        str = str & Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = str '& " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Convert.ToString(Read_Buf(0), 16) & "." & Convert.ToString(Read_Buf(1), 16) & "." & Convert.ToString(Read_Buf(2), 16) & " " & "-" & " " & Convert.ToString(Read_Buf(3), 16) & "." & Convert.ToString(Read_Buf(4), 16) & "." & Convert.ToString(Read_Buf(5), 16)

                    ElseIf PMBus_MFR_Struct(Arr_Loc).Command = &HF6 Then 'BL_REVISION
                        str = Convert.ToString(Read_Buf(0), 16) & " "
                        str = str & Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView4.Rows(Arr_Loc).Cells(2).Value() = str ' & " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView4.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Convert.ToString(Read_Buf(0), 16) & "." & Convert.ToString(Read_Buf(1), 16) & "." & Convert.ToString(Read_Buf(2), 16) & " " & "-" & " " & Convert.ToString(Read_Buf(3), 16) & "." & Convert.ToString(Read_Buf(4), 16) & "." & Convert.ToString(Read_Buf(5), 16)

                    ElseIf PMBus_MFR_Struct(Arr_Loc).Command = &HF7 Then 'QCS_FW_REVISION
                        str = Convert.ToString(Read_Buf(1), 16) & " "
                        str = str & Convert.ToString(Read_Buf(2), 16) & " "
                        str = str & Convert.ToString(Read_Buf(3), 16) & " "
                        str = str & Convert.ToString(Read_Buf(4), 16) & " "
                        str = str & Convert.ToString(Read_Buf(5), 16) & " "
                        str = str & Convert.ToString(Read_Buf(6), 16) & " "
                        DataGridView_MFR.Rows(Arr_Loc).Cells(2).Value() = Convert.ToString(Read_Buf(0), 16) & " " & str & " " & Convert.ToString(Read_Buf(7), 16)
                        DataGridView_MFR.Rows(Arr_Loc).Cells(3).Value() = str
                        DataGridView_MFR.Rows(Arr_Loc).Cells(4).Value() = str 'ascii.GetString(Read_Buf, 1, 7)
                    End If
#End If
                End If

            End If
        Next
    End Sub

    Private Sub Update_Log_to_file(ByVal Null_Data As Byte)
        If Not Data_Arr_Pntr = 0 Then
            Dim i As UInteger = 0
            For i = 0 To Data_Arr_Pntr - 1
                Dim bytes() As Byte
                bytes = Encoding.ASCII.GetBytes(Data_Str(i) & vbCrLf)
                My.Computer.FileSystem.WriteAllBytes(Log_File_Name, bytes, True)
            Next
            Data_Arr_Pntr = 0
        End If
    End Sub


    Private Sub Update_FRU_to_file(ByVal Null_Data As Byte)
        If Not Data_Arr_Pntr = 0 Then
            Dim i As UInteger = 0
            For i = 0 To Data_Arr_Pntr - 1
                Dim bytes() As Byte
                bytes = Encoding.ASCII.GetBytes(Data_Str(i) & vbCrLf)
                My.Computer.FileSystem.WriteAllBytes(FRU_File_Name, bytes, True)
            Next
            Data_Arr_Pntr = 0
        End If
    End Sub

#End Region

#If remove Then
    Private Sub Update_Pmbus_Status(ByVal Null_Data As Byte)
        If Pic_Kit_Error = False Then
            'Update Pmbus Status Bits
            Dim Bits As Byte

            'Status Word - LSB
            Dim str As String = DataGridView1.Rows(2).Cells(3).Value()
            str = Mid(str, 3, 2)
            Bits = Convert.ToByte(str, 16)

            If (Bits And &H80) = &H80 Then
                PictureBox8.BackColor = Color.Red
            Else
                PictureBox8.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox7.BackColor = Color.Red
            Else
                PictureBox7.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox6.BackColor = Color.Red
            Else
                PictureBox6.BackColor = Color.Gainsboro
            End If

            If (Bits And &H10) = &H10 Then
                PictureBox5.BackColor = Color.Red
            Else
                PictureBox5.BackColor = Color.Gainsboro
            End If

            If (Bits And &H8) = &H8 Then
                PictureBox4.BackColor = Color.Red
            Else
                PictureBox4.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox3.BackColor = Color.Red
            Else
                PictureBox3.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox2.BackColor = Color.Red
            Else
                PictureBox2.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox1.BackColor = Color.Red
            Else
                PictureBox1.BackColor = Color.Gainsboro
            End If



#If 1 Then
            'Status Word - LSB
            If (Bits And &H80) = &H80 Then
                PictureBox_Busy.BackColor = Color.Red
            Else
                PictureBox_Busy.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox_OFF.BackColor = Color.Red
            Else
                PictureBox_OFF.BackColor = Color.Gainsboro
            End If
            If (Bits And &H20) Then
                PictureBox_VOUT_OV.BackColor = Color.Red
            Else
                PictureBox_VOUT_OV.BackColor = Color.Gainsboro
            End If

            If (Bits And &H10) = &H10 Then
                PictureBox_IOUT_OC.BackColor = Color.Red
            Else
                PictureBox_IOUT_OC.BackColor = Color.Gainsboro
            End If

            If (Bits And &H8) = &H8 Then
                PictureBox106.BackColor = Color.Red
            Else
                PictureBox106.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox139.BackColor = Color.Red
            Else
                PictureBox139.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox122.BackColor = Color.Red
            Else
                PictureBox122.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox144.BackColor = Color.Red
            Else
                PictureBox144.BackColor = Color.Gainsboro
            End If
#End If



            'Status Word - MSB
            str = DataGridView1.Rows(2).Cells(3).Value()
            str = Mid(str, 1, 2)
            Bits = Convert.ToByte(str, 16)

            If (Bits And &H80) = &H80 Then
                PictureBox16.BackColor = Color.Red
            Else
                PictureBox16.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox14.BackColor = Color.Red
            Else
                PictureBox14.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox12.BackColor = Color.Red
            Else
                PictureBox12.BackColor = Color.Gainsboro
            End If

            If (Bits And &H10) = &H10 Then
                PictureBox10.BackColor = Color.Red
            Else
                PictureBox10.BackColor = Color.Gainsboro
            End If

            If (Bits And &H8) = &H8 Then
                PictureBox15.BackColor = Color.Red
            Else
                PictureBox15.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox11.BackColor = Color.Red
            Else
                PictureBox11.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox13.BackColor = Color.Red
            Else
                PictureBox13.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox9.BackColor = Color.Red
            Else
                PictureBox9.BackColor = Color.Gainsboro
            End If



            'Status Word - MSB

#If 1 Then
            If (Bits And &H80) = &H80 Then
                PictureBox90.BackColor = Color.Red
            Else
                PictureBox90.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox103.BackColor = Color.Red
            Else
                PictureBox103.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox119.BackColor = Color.Red
            Else
                PictureBox119.BackColor = Color.Gainsboro
            End If

            If (Bits And &H10) = &H10 Then
                PictureBox135.BackColor = Color.Red
            Else
                PictureBox135.BackColor = Color.Gainsboro
            End If

            If (Bits And &H8) = &H8 Then
                PictureBox95.BackColor = Color.Red
            Else
                PictureBox95.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox127.BackColor = Color.Red
            Else
                PictureBox127.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox111.BackColor = Color.Red
            Else
                PictureBox111.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox141.BackColor = Color.Red
            Else
                PictureBox141.BackColor = Color.Gainsboro
            End If
#End If

            'Status VOUT
            Bits = Convert.ToUInt16(DataGridView1.Rows(3).Cells(3).Value(), 16)

            If (Bits And &H80) = &H80 Then
                PictureBox24.BackColor = Color.Red
            Else
                PictureBox24.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox22.BackColor = Color.Red
            Else
                PictureBox22.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox20.BackColor = Color.Red
            Else
                PictureBox20.BackColor = Color.Gainsboro
            End If

            If (Bits And &H10) = &H10 Then
                PictureBox18.BackColor = Color.Red
            Else
                PictureBox18.BackColor = Color.Gainsboro
            End If

            If (Bits And &H8) = &H8 Then
                PictureBox23.BackColor = Color.Red
            Else
                PictureBox23.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox19.BackColor = Color.Red
            Else
                PictureBox19.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox21.BackColor = Color.Red
            Else
                PictureBox21.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox17.BackColor = Color.Red
            Else
                PictureBox17.BackColor = Color.Gainsboro
            End If
#If 1 Then

            'Status VOUT

            If (Bits And &H80) = &H80 Then
                PictureBox89.BackColor = Color.Red
            Else
                PictureBox89.BackColor = Color.Gainsboro
            End If
#If 0 Then
            If (Bits And &H40) = &H40 Then
                PictureBox97.BackColor = Color.Red
            Else
                PictureBox97.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox20.BackColor = Color.Red
            Else
                PictureBox20.BackColor = Color.Gainsboro
            End If
#End If
            If (Bits And &H10) = &H10 Then
                PictureBox129.BackColor = Color.Red
            Else
                PictureBox129.BackColor = Color.Gainsboro
            End If
#If 0 Then

            If (Bits And &H8) = &H8 Then
                PictureBox23.BackColor = Color.Red
            Else
                PictureBox23.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox19.BackColor = Color.Red
            Else
                PictureBox19.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox21.BackColor = Color.Red
            Else
                PictureBox21.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox17.BackColor = Color.Red
            Else
                PictureBox17.BackColor = Color.Gainsboro
            End If
#End If

#End If

            'Status IOUT
            Bits = Convert.ToUInt16(DataGridView1.Rows(4).Cells(3).Value(), 16)

            If (Bits And &H80) = &H80 Then
                PictureBox32.BackColor = Color.Red
            Else
                PictureBox32.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox30.BackColor = Color.Red
            Else
                PictureBox30.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox28.BackColor = Color.Red
            Else
                PictureBox28.BackColor = Color.Gainsboro
            End If

            If (Bits And &H10) = &H10 Then
                PictureBox26.BackColor = Color.Red
            Else
                PictureBox26.BackColor = Color.Gainsboro
            End If

            If (Bits And &H8) = &H8 Then
                PictureBox31.BackColor = Color.Red
            Else
                PictureBox31.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox27.BackColor = Color.Red
            Else
                PictureBox27.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox29.BackColor = Color.Red
            Else
                PictureBox29.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox25.BackColor = Color.Red
            Else
                PictureBox25.BackColor = Color.Gainsboro
            End If


#If 1 Then
            If (Bits And &H80) = &H80 Then
                PictureBox81.BackColor = Color.Red
            Else
                PictureBox81.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox92.BackColor = Color.Red
            Else
                PictureBox92.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox108.BackColor = Color.Red
            Else
                PictureBox108.BackColor = Color.Gainsboro
            End If
#If 0 Then

            If (Bits And &H10) = &H10 Then
                PictureBox26.BackColor = Color.Red
            Else
                PictureBox26.BackColor = Color.Gainsboro
            End If
#End If

            If (Bits And &H8) = &H8 Then
                PictureBox84.BackColor = Color.Red
            Else
                PictureBox84.BackColor = Color.Gainsboro
            End If
#If 0 Then

            If (Bits And &H4) = &H4 Then
                PictureBox27.BackColor = Color.Red
            Else
                PictureBox27.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox29.BackColor = Color.Red
            Else
                PictureBox29.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox25.BackColor = Color.Red
            Else
                PictureBox25.BackColor = Color.Gainsboro
            End If
#End If

#End If

            If Page_sel = 0 Then

                'Status Input
                Bits = Convert.ToUInt16(DataGridView1.Rows(5).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox56.BackColor = Color.Red
                Else
                    PictureBox56.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox48.BackColor = Color.Red
                Else
                    PictureBox48.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox40.BackColor = Color.Red
                Else
                    PictureBox40.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox34.BackColor = Color.Red
                Else
                    PictureBox34.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox52.BackColor = Color.Red
                Else
                    PictureBox52.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox36.BackColor = Color.Red
                Else
                    PictureBox36.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox44.BackColor = Color.Red
                Else
                    PictureBox44.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox33.BackColor = Color.Red
                Else
                    PictureBox33.BackColor = Color.Gainsboro
                End If


#If 1 Then

                If (Bits And &H80) = &H80 Then
                    PictureBox96.BackColor = Color.Red
                Else
                    PictureBox96.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox112.BackColor = Color.Red
                Else
                    PictureBox112.BackColor = Color.Gainsboro
                End If
#If 0 Then

                If (Bits And &H20) Then
                    PictureBox40.BackColor = Color.Red
                Else
                    PictureBox40.BackColor = Color.Gainsboro
                End If
#End If
                If (Bits And &H10) = &H10 Then
                    PictureBox140.BackColor = Color.Red
                Else
                    PictureBox140.BackColor = Color.Gainsboro
                End If
#If 0 Then
                If (Bits And &H8) = &H8 Then
                    PictureBox52.BackColor = Color.Red
                Else
                    PictureBox52.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox36.BackColor = Color.Red
                Else
                    PictureBox36.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox44.BackColor = Color.Red
                Else
                    PictureBox44.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox33.BackColor = Color.Red
                Else
                    PictureBox33.BackColor = Color.Gainsboro
                End If
#End If

#End If

                'Status Temperature
                Bits = Convert.ToUInt16(DataGridView1.Rows(6).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox60.BackColor = Color.Red
                Else
                    PictureBox60.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox54.BackColor = Color.Red
                Else
                    PictureBox54.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox46.BackColor = Color.Red
                Else
                    PictureBox46.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox38.BackColor = Color.Red
                Else
                    PictureBox38.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox58.BackColor = Color.Red
                Else
                    PictureBox58.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox42.BackColor = Color.Red
                Else
                    PictureBox42.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox50.BackColor = Color.Red
                Else
                    PictureBox50.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox35.BackColor = Color.Red
                Else
                    PictureBox35.BackColor = Color.Gainsboro
                End If

#If 1 Then
                If (Bits And &H80) = &H80 Then
                    PictureBox88.BackColor = Color.Red
                Else
                    PictureBox88.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox99.BackColor = Color.Red
                Else
                    PictureBox99.BackColor = Color.Gainsboro
                End If
#If 0 Then
                If (Bits And &H20) Then
                    PictureBox46.BackColor = Color.Red
                Else
                    PictureBox46.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox38.BackColor = Color.Red
                Else
                    PictureBox38.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox58.BackColor = Color.Red
                Else
                    PictureBox58.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox42.BackColor = Color.Red
                Else
                    PictureBox42.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox50.BackColor = Color.Red
                Else
                    PictureBox50.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox35.BackColor = Color.Red
                Else
                    PictureBox35.BackColor = Color.Gainsboro
                End If
#End If
#End If

                'Status CML
                Bits = Convert.ToUInt16(DataGridView1.Rows(7).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox62.BackColor = Color.Red
                Else
                    PictureBox62.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox57.BackColor = Color.Red
                Else
                    PictureBox57.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox49.BackColor = Color.Red
                Else
                    PictureBox49.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox41.BackColor = Color.Red
                Else
                    PictureBox41.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox61.BackColor = Color.Red
                Else
                    PictureBox61.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox45.BackColor = Color.Red
                Else
                    PictureBox45.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox53.BackColor = Color.Red
                Else
                    PictureBox53.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox37.BackColor = Color.Red
                Else
                    PictureBox37.BackColor = Color.Gainsboro
                End If


#If 1 Then
                If (Bits And &H80) = &H80 Then
                    PictureBox82.BackColor = Color.Red
                Else
                    PictureBox82.BackColor = Color.Gainsboro
                End If
#If 0 Then

                If (Bits And &H40) = &H40 Then
                    PictureBox57.BackColor = Color.Red
                Else
                    PictureBox57.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox49.BackColor = Color.Red
                Else
                    PictureBox49.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox41.BackColor = Color.Red
                Else
                    PictureBox41.BackColor = Color.Gainsboro
                End If
#End If
                If (Bits And &H8) = &H8 Then
                    PictureBox85.BackColor = Color.Red
                Else
                    PictureBox85.BackColor = Color.Gainsboro
                End If
#If 0 Then

                If (Bits And &H4) = &H4 Then
                    PictureBox45.BackColor = Color.Red
                Else
                    PictureBox45.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox53.BackColor = Color.Red
                Else
                    PictureBox53.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox37.BackColor = Color.Red
                Else
                    PictureBox37.BackColor = Color.Gainsboro
                End If
#End If
#End If

                'Status Fan 1 & 2
                Bits = Convert.ToUInt16(DataGridView1.Rows(10).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox168.BackColor = Color.Red
                Else
                    PictureBox168.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox166.BackColor = Color.Red
                Else
                    PictureBox166.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox164.BackColor = Color.Red
                Else
                    PictureBox164.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox162.BackColor = Color.Red
                Else
                    PictureBox162.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox167.BackColor = Color.Red
                Else
                    PictureBox167.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox163.BackColor = Color.Red
                Else
                    PictureBox163.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox165.BackColor = Color.Red
                Else
                    PictureBox165.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox161.BackColor = Color.Red
                Else
                    PictureBox161.BackColor = Color.Gainsboro
                End If

                'Status Other
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(8).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox39.BackColor = Color.Red
                Else
                    PictureBox39.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox47.BackColor = Color.Red
                Else
                    PictureBox47.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox55.BackColor = Color.Red
                Else
                    PictureBox55.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox63.BackColor = Color.Red
                Else
                    PictureBox63.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox43.BackColor = Color.Red
                Else
                    PictureBox43.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox59.BackColor = Color.Red
                Else
                    PictureBox59.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox51.BackColor = Color.Red
                Else
                    PictureBox51.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox64.BackColor = Color.Red
                Else
                    PictureBox64.BackColor = Color.Gainsboro
                End If

                'Status MFR
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(9).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox169.BackColor = Color.Red
                Else
                    PictureBox169.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox171.BackColor = Color.Red
                Else
                    PictureBox171.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox173.BackColor = Color.Red
                Else
                    PictureBox173.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox175.BackColor = Color.Red
                Else
                    PictureBox175.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox170.BackColor = Color.Red
                Else
                    PictureBox170.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox174.BackColor = Color.Red
                Else
                    PictureBox174.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox172.BackColor = Color.Red
                Else
                    PictureBox172.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox176.BackColor = Color.Red
                Else
                    PictureBox176.BackColor = Color.Gainsboro
                End If

            Else
                'Status Input
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(5).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox56.BackColor = Color.Red
                Else
                    PictureBox56.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox48.BackColor = Color.Red
                Else
                    PictureBox48.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox40.BackColor = Color.Red
                Else
                    PictureBox40.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox34.BackColor = Color.Red
                Else
                    PictureBox34.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox52.BackColor = Color.Red
                Else
                    PictureBox52.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox36.BackColor = Color.Red
                Else
                    PictureBox36.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox44.BackColor = Color.Red
                Else
                    PictureBox44.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox33.BackColor = Color.Red
                Else
                    PictureBox33.BackColor = Color.Gainsboro
                End If

                'Status Temperature
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(6).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox60.BackColor = Color.Red
                Else
                    PictureBox60.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox54.BackColor = Color.Red
                Else
                    PictureBox54.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox46.BackColor = Color.Red
                Else
                    PictureBox46.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox38.BackColor = Color.Red
                Else
                    PictureBox38.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox58.BackColor = Color.Red
                Else
                    PictureBox58.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox42.BackColor = Color.Red
                Else
                    PictureBox42.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox50.BackColor = Color.Red
                Else
                    PictureBox50.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox35.BackColor = Color.Red
                Else
                    PictureBox35.BackColor = Color.Gainsboro
                End If

                'Status CML
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(7).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox62.BackColor = Color.Red
                Else
                    PictureBox62.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox57.BackColor = Color.Red
                Else
                    PictureBox57.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox49.BackColor = Color.Red
                Else
                    PictureBox49.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox41.BackColor = Color.Red
                Else
                    PictureBox41.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox61.BackColor = Color.Red
                Else
                    PictureBox61.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox45.BackColor = Color.Red
                Else
                    PictureBox45.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox53.BackColor = Color.Red
                Else
                    PictureBox53.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox37.BackColor = Color.Red
                Else
                    PictureBox37.BackColor = Color.Gainsboro
                End If

                'Status Fan 1 & 2
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(10).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox168.BackColor = Color.Red
                Else
                    PictureBox168.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox166.BackColor = Color.Red
                Else
                    PictureBox166.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox164.BackColor = Color.Red
                Else
                    PictureBox164.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox162.BackColor = Color.Red
                Else
                    PictureBox162.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox167.BackColor = Color.Red
                Else
                    PictureBox167.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox163.BackColor = Color.Red
                Else
                    PictureBox163.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox165.BackColor = Color.Red
                Else
                    PictureBox165.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox161.BackColor = Color.Red
                Else
                    PictureBox161.BackColor = Color.Gainsboro
                End If

                'Status Other
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(8).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox39.BackColor = Color.Red
                Else
                    PictureBox39.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox47.BackColor = Color.Red
                Else
                    PictureBox47.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox55.BackColor = Color.Red
                Else
                    PictureBox55.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox63.BackColor = Color.Red
                Else
                    PictureBox63.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox43.BackColor = Color.Red
                Else
                    PictureBox43.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox59.BackColor = Color.Red
                Else
                    PictureBox59.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox51.BackColor = Color.Red
                Else
                    PictureBox51.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox64.BackColor = Color.Red
                Else
                    PictureBox64.BackColor = Color.Gainsboro
                End If


#If 1 Then
                'Status Other
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(8).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox83.BackColor = Color.Red
                Else
                    PictureBox83.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox94.BackColor = Color.Red
                Else
                    PictureBox94.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox110.BackColor = Color.Red
                Else
                    PictureBox110.BackColor = Color.Gainsboro
                End If
#If 0 Then

                If (Bits And &H10) = &H10 Then
                    PictureBox63.BackColor = Color.Red
                Else
                    PictureBox63.BackColor = Color.Gainsboro
                End If
#End If
                If (Bits And &H8) = &H8 Then
                    PictureBox86.BackColor = Color.Red
                Else
                    PictureBox86.BackColor = Color.Gainsboro
                End If
#If 0 Then
                If (Bits And &H4) = &H4 Then
                    PictureBox59.BackColor = Color.Red
                Else
                    PictureBox59.BackColor = Color.Gainsboro
                End If
#End If
                If (Bits And &H2) = &H2 Then
                    PictureBox102.BackColor = Color.Red
                Else
                    PictureBox102.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox134.BackColor = Color.Red
                Else
                    PictureBox134.BackColor = Color.Gainsboro
                End If


#End If




#If 1 Then

                'Status Other
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(8).Cells(3).Value(), 16)
#If 0 Then
                If (Bits And &H80) = &H80 Then
                    PictureBox39.BackColor = Color.Red
                Else
                    PictureBox39.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox47.BackColor = Color.Red
                Else
                    PictureBox47.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox55.BackColor = Color.Red
                Else
                    PictureBox55.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox63.BackColor = Color.Red
                Else
                    PictureBox63.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox43.BackColor = Color.Red
                Else
                    PictureBox43.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox59.BackColor = Color.Red
                Else
                    PictureBox59.BackColor = Color.Gainsboro
                End If
#End If
                If (Bits And &H2) = &H2 Then
                    PictureBox76.BackColor = Color.Red
                Else
                    PictureBox76.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox80.BackColor = Color.Red
                Else
                    PictureBox80.BackColor = Color.Gainsboro
                End If
#End If

                'Status MFR
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(9).Cells(3).Value(), 16)

                If (Bits And &H80) = &H80 Then
                    PictureBox169.BackColor = Color.Red
                Else
                    PictureBox169.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox171.BackColor = Color.Red
                Else
                    PictureBox171.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox173.BackColor = Color.Red
                Else
                    PictureBox173.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox175.BackColor = Color.Red
                Else
                    PictureBox175.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox170.BackColor = Color.Red
                Else
                    PictureBox170.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox174.BackColor = Color.Red
                Else
                    PictureBox174.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox172.BackColor = Color.Red
                Else
                    PictureBox172.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox176.BackColor = Color.Red
                Else
                    PictureBox176.BackColor = Color.Gainsboro
                End If

#If 1 Then
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(8).Cells(3).Value(), 16)
#If 0 Then

                If (Bits And &H80) = &H80 Then
                    PictureBox39.BackColor = Color.Red
                Else
                    PictureBox39.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox47.BackColor = Color.Red
                Else
                    PictureBox47.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox55.BackColor = Color.Red
                Else
                    PictureBox55.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox63.BackColor = Color.Red
                Else
                    PictureBox63.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox43.BackColor = Color.Red
                Else
                    PictureBox43.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox59.BackColor = Color.Red
                Else
                    PictureBox59.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox51.BackColor = Color.Red
                Else
                    PictureBox51.BackColor = Color.Gainsboro
                End If
#End If
                If (Bits And &H1) = &H1 Then
                    PictureBox72.BackColor = Color.Red
                Else
                    PictureBox72.BackColor = Color.Gainsboro
                End If

                'Status MFR
                Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(9).Cells(3).Value(), 16)
#If 0 Then

                If (Bits And &H80) = &H80 Then
                    PictureBox169.BackColor = Color.Red
                Else
                    PictureBox169.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox171.BackColor = Color.Red
                Else
                    PictureBox171.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox173.BackColor = Color.Red
                Else
                    PictureBox173.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox175.BackColor = Color.Red
                Else
                    PictureBox175.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox170.BackColor = Color.Red
                Else
                    PictureBox170.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox174.BackColor = Color.Red
                Else
                    PictureBox174.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox172.BackColor = Color.Red
                Else
                    PictureBox172.BackColor = Color.Gainsboro
                End If
#End If
                If (Bits And &H1) = &H1 Then
                    PictureBox72.BackColor = Color.Red
                Else
                    PictureBox72.BackColor = Color.Gainsboro
                End If
#End If

            End If

        End If

    End Sub
#End If
    Private Sub Update_Pmbus_Status_1()

    End Sub

    Private Sub Update_Pmbus_Data(ByVal Null_Data As Byte)
        Dim Arr_Len As Byte = PMBus_Data_Struct.Length
        Dim str1 As String

        For Arr_Loc = 0 To Arr_Len - 1

            If PMBus_Data_Struct(Arr_Loc).RW_Length = 1 Then

                If Page_sel = 1 Then

                    If PMBus_Data_Struct(Arr_Loc).Command = &H0 Or PMBus_Data_Struct(Arr_Loc).Command = &H20 Or PMBus_Data_Struct(Arr_Loc).Command = &H78 Or PMBus_Data_Struct(Arr_Loc).Command = &H7A Or PMBus_Data_Struct(Arr_Loc).Command = &H7B Then

                        DataGridView1.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_Data_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView1.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView1.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        DataGridView1.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView1.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView1.Rows(Arr_Loc).Cells(4).Value() = "-"
                    End If
                Else
                    DataGridView1.Rows(Arr_Loc).Cells(2).Value() = Read_Byte(PMBus_Data_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                    DataGridView1.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    DataGridView1.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                End If

                'If DataGridView1.Rows(0).Cells(3).Value() = &H0 Then
                '    Page_sel = 0
                'ElseIf DataGridView1.Rows(0).Cells(3).Value() = &H1 Then
                '    Page_sel = 1
                'End If

            ElseIf PMBus_Data_Struct(Arr_Loc).RW_Length = 2 Then

                If Page_sel = 1 Then

                    If PMBus_Data_Struct(Arr_Loc).Command = &H79 Or PMBus_Data_Struct(Arr_Loc).Command = &H8B Or PMBus_Data_Struct(Arr_Loc).Command = &H8C Or PMBus_Data_Struct(Arr_Loc).Command = &H96 Then

                        If PMBus_Data_Struct(Arr_Loc).Data = False Then
                            DataGridView1.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_Data_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                            DataGridView1.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView1.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        Else
                            DataGridView1.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Pmb(PMBus_Data_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                            DataGridView1.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                            DataGridView1.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                        End If
                    Else
                        DataGridView1.Rows(Arr_Loc).Cells(2).Value() = "-"
                        DataGridView1.Rows(Arr_Loc).Cells(3).Value() = "-"
                        DataGridView1.Rows(Arr_Loc).Cells(4).Value() = "-"
                    End If

                Else

                    If PMBus_Data_Struct(Arr_Loc).Data = False Then
                        DataGridView1.Rows(Arr_Loc).Cells(2).Value() = Read_Word(PMBus_Data_Struct(Arr_Loc).Command) 'Returns All Data With PEC
                        DataGridView1.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView1.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    Else
                        DataGridView1.Rows(Arr_Loc).Cells(2).Value() = Read_Linear_Word_Pmb(PMBus_Data_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                        DataGridView1.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView1.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
                    End If

                End If

            ElseIf PMBus_Data_Struct(Arr_Loc).RW_Length > 2 Then

                If Page_sel = 1 Then

                    'If PMBus_Data_Struct(Arr_Loc).Command = &H86 Or PMBus_Data_Struct(Arr_Loc).Command = &H87 Then

                    'If PMBus_Data_Struct(Arr_Loc).Data = False Then
                    '    DataGridView1.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_Data_Struct(Arr_Loc).Command, PMBus_Data_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                    '    DataGridView1.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                    '    DataGridView1.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                    '    str1 = Convert.ToString(Read_Buf(1), 16) & " "
                    '    str1 = str1 & Convert.ToString(Read_Buf(2), 16) & " "
                    '    str1 = str1 & Convert.ToString(Read_Buf(3), 16) & " "
                    '    str1 = str1 & Convert.ToString(Read_Buf(4), 16) & " "
                    '    str1 = str1 & Convert.ToString(Read_Buf(5), 16) & " "
                    '    str1 = str1 & Convert.ToString(Read_Buf(6), 16) & " "
                    '    DataGridView1.Rows(Arr_Loc).Cells(3).Value() = str1 & Convert.ToString(Read_Buf(7), 16)
                    '    DataGridView1.Rows(Arr_Loc).Cells(4).Value() = (Read_Buf(1) + (Read_Buf(2) * 256)) / ((Read_Buf(6) * 65536) + (Read_Buf(5) * 256) + Read_Buf(4))

                    'Else
                    '    DataGridView1.Rows(Arr_Loc).Cells(2).Value() = "-" 'Read_Linear_Word_Pmb(PMBus_Data_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                    '    DataGridView1.Rows(Arr_Loc).Cells(3).Value() = "-" 'Pmb_Hex_Data
                    '    DataGridView1.Rows(Arr_Loc).Cells(4).Value() = "-" 'Pmb_Act_Data
                    'End If
                    'Else
                    DataGridView1.Rows(Arr_Loc).Cells(2).Value() = "-"
                    DataGridView1.Rows(Arr_Loc).Cells(3).Value() = "-"
                    DataGridView1.Rows(Arr_Loc).Cells(4).Value() = "-"
                    'End If

                Else

                    If PMBus_Data_Struct(Arr_Loc).Data = False Then

                        DataGridView1.Rows(Arr_Loc).Cells(2).Value() = Read_Block(PMBus_Data_Struct(Arr_Loc).Command, PMBus_Data_Struct(Arr_Loc).RW_Length) 'Returns All Data With PEC
                        DataGridView1.Rows(Arr_Loc).Cells(3).Value() = Pmb_Hex_Data
                        DataGridView1.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data

                        If PMBus_Data_Struct(Arr_Loc).Command = &H86 Or PMBus_Data_Struct(Arr_Loc).Command = &H87 Then

                            str1 = Convert.ToString(Read_Buf(1), 16) & " "
                            str1 = str1 & Convert.ToString(Read_Buf(2), 16) & " "
                            str1 = str1 & Convert.ToString(Read_Buf(3), 16) & " "
                            str1 = str1 & Convert.ToString(Read_Buf(4), 16) & " "
                            str1 = str1 & Convert.ToString(Read_Buf(5), 16) & " "
                            str1 = str1 & Convert.ToString(Read_Buf(6), 16) & " "
                            DataGridView1.Rows(Arr_Loc).Cells(3).Value() = str1 & Convert.ToString(Read_Buf(7), 16)
                            '  DataGridView1.Rows(Arr_Loc).Cells(4).Value() = (Read_Buf(1) + (Read_Buf(2) * 256)) / ((Read_Buf(6) * 65536) + (Read_Buf(5) * 256) + Read_Buf(4))

                        End If

                    Else
                        DataGridView1.Rows(Arr_Loc).Cells(2).Value() = "-" 'Read_Linear_Word_Pmb(PMBus_Data_Struct(Arr_Loc).Command, Arr_Loc) 'Returns All Data With PEC
                        DataGridView1.Rows(Arr_Loc).Cells(3).Value() = "-" 'Pmb_Hex_Data
                        DataGridView1.Rows(Arr_Loc).Cells(4).Value() = "-" 'Pmb_Act_Data
                    End If

                End If

            End If

        Next





    End Sub
#End Region

#Region "Internal Data Controls & Functions"

#Region "Controls"
#End Region

#Region "Functions"
    Private Sub Button8_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        'Unlock Debug Mode
        Write_Buf(0) = &H55
        Write_Buf(1) = &H4C
        Write_Word(&HFD)
    End Sub

    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        'lock Debug Mode
        Write_Buf(0) = &HAA
        Write_Buf(1) = &HAA
        Write_Word(&HFD)
    End Sub

    Private Sub Process_Cal_Data(ByVal Null_Data As Byte)
        If x1 = x2 Then
            k = 0
            b = 0 'y1
        Else
            k = (y2 - y1) / (x2 - x1)

        End If

        Gain_Temp = (RANGE * k)
        If Gain_Temp < 0 Then
            Gain = (65536 + Gain_Temp)
        Else
            Gain = Gain_Temp
        End If

        Cal_temp = (Gain * x1 / RANGE)
        b = (y1 - Cal_temp)

        offset_temp = b
        If offset_temp < 0 Then
            Offset = (65536 + offset_temp)
        Else
            Offset = offset_temp
        End If
    End Sub
    Private Sub Process_Cal_Current_Share(ByVal Null_Data As Byte)

        If x1 = x2 Then
            k = 4096
            b = 0 'y1
        Else
            k = (z2 - z1) / (y2 - y1)
            b = ((z1 - y1) * x2 - (z2 - y2) * x1) / (y2 - y1)
        End If

        Gain_Temp = (RANGE * k)
        If Gain_Temp < 0 Then
            Gain = (65536 + Gain_Temp)
        Else
            Gain = Gain_Temp
        End If

        offset_temp = b
        If offset_temp < 0 Then
            Offset = (65536 + offset_temp)
        Else
            Offset = offset_temp
        End If
    End Sub

    Private Sub Process_Cal_Data1(ByVal Null_Data As Byte)

        If x1 = x2 Then
            k = 0
            b = 0 'y1
        Else
            k = (y2 - y1) / (x2 - x1)
            Cal_temp = (k * x1)
            b = (y1 - Cal_temp)
        End If

        Gain_Temp = (RANGE * k)
        Gain_Temp = Gain_Temp - RANGE
        If Gain_Temp < 0 Then
            Gain = (256 + Gain_Temp)
        Else
            Gain = Gain_Temp
        End If
        'If RANGE > Gain_Temp Then
        '    Gain_Temp = RANGE - Gain_Temp
        'Else
        '    Gain_Temp = Gain_Temp - RANGE
        'End If

        offset_temp = b * PMBus_Factor
        If offset_temp < 0 Then
            Offset = (65535 + offset_temp)
        Else
            Offset = offset_temp
        End If

    End Sub
#End Region
#End Region

#Region "Timer Interrupts"
    Private Sub Timer1_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Timer1.Tick
        Dim Return_Str As String = "0000"
        Dim time As DateTime = DateTime.Now
        Dim format As String = "d MMM yyyy HH:mm:ss"
        If Capture_Data = True Then
            Capture_Data_Delay += 1
        End If
        If Hardware_Detected = 1 Then


            'If Win_I2C_Error = True And Hardware_Selection = 1 Then
            'Else
            If Pic_Kit_Error = True And Hardware_Selection = 2 Then
                    Err_Rec_Count += 1
                    If Err_Rec_Count > 200 Then ' Delay 0.5 Sec
                        Pic_Kit_Error = False
                        '   Init_PKSA(0)
                        PICkitS.Device.Reset_Control_Block()
                        PICkitS.Device.Reset_Control_Block()
                        PICkitS.Device.Reset_Control_Block()
                        Append_Text1("Pickit Control Block Resetted" & vbCrLf)
                        Err_Rec_Count = 0
                        RichTextBox1.BackColor = Color.White
                        'Reset all status after Pickit Reset

                        'Read_Pri_Flag = False
                    ElseIf Err_Rec_Count = 1 Then
                        RichTextBox1.BackColor = Color.Khaki
                    End If
                ElseIf Capture_Data = True And Capture_Data_Delay > 500 Then  'Read Every 10 Second
                    Capture_Data_Delay = 0
                    Dim T_Str As String = "'" & time.ToString(format)
                    Dim str As String = ""

                    str = Read_Linear_Word_Pmb(&H8D, 13)
                    T_Str = T_Str & "," & Pmb_Act_Data
                    str = Read_Linear_Word_Pmb(&H8F, 14)
                    T_Str = T_Str & "," & Pmb_Act_Data
                    str = Read_Linear_Word_Pmb(&H8E, 15)
                    T_Str = T_Str & "," & Pmb_Act_Data
                    str = Read_Linear_Word_Pmb(&H90, 16)
                    T_Str = T_Str & "," & Pmb_Act_Data

                    Data_Str(Data_Arr_Pntr) = T_Str

                    Data_Arr_Pntr += 1
                    Capture_Data_Pntr += 1

                    If Data_Arr_Pntr >= 60 Then 'Every 1 min - 60 Captures
                        Update_Log_to_file(0)
                    End If

                    If Capture_Data_Pntr >= 360 Then
                    Capture_Data = False
                End If

                ElseIf Query = True Then

                Else
                    Err_Rec_Count = 0
                If Poll_Button_Latched = True Then
                    Poll_Dly_Count += 1
                    Dim Poll_Delay As Byte = NumericUpDown7.Value / 10  ' 10mS delay
                    If Poll_Dly_Count >= Poll_Delay Then
                        Read_All(0)
                        Poll_Dly_Count = 0
                    End If
                End If
            End If
        End If
    End Sub
    Private Sub Timer2_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Timer2.Tick
        Dim Win_I2C_Count As UShort
        Dim PKSA_Count As UShort

        If Hardware_Detected = 0 Then ' Hardware not detcted yet
            Win_I2C_Count = 0          'GetNumberOfDevices()
            PKSA_Count = PICkitS.Device.How_Many_PICkitSerials_Are_Attached()

            If Win_I2C_Count >= 1 And (Hardware_Selection = 0 Or Hardware_Selection = 1) Then
            ElseIf PKSA_Count = 1 And (Hardware_Selection = 0 Or Hardware_Selection = 2) Then
                If Init_PKSA(0) = True Then
                    Hardware_Detected = 1
                    Hardware_Selection = 2
                    ' RadioButton16.Enabled = False
                    ToolStripStatusLabel1.Text = " PKSA Hardware Detected"
                    ToolStripStatusLabel1.BackColor = Color.YellowGreen
                    TabControl1.Enabled = True
                    PICkitConnect.Enabled = True
                    Poll.Enabled = True
                    ReadOnce.Enabled = True
                Else
                    ToolStripStatusLabel1.Text = " PKSA Hardware Not Detected"
#If 1 Then
                    ToolStripStatusLabel1.BackColor = Color.Red
                    TabControl1.Enabled = False
                    PICkitConnect.Enabled = False
                    Poll.Enabled = False
                    ReadOnce.Enabled = False
#End If
                End If
            Else
                ToolStripStatusLabel1.Text = "Hardware Not Detected"
                ToolStripStatusLabel1.BackColor = Color.Red
#If remove Then
                
                TabControl1.Enabled = False
                Poll.Enabled = False
                ReadOnce.Enabled = False
#End If
            End If
        ElseIf Hardware_Detected = 1 Then
            If Hardware_Selection = 1 And (Poll_Button_Latched = False Or Win_I2C_Error = False) Then
            ElseIf Hardware_Selection = 2 And (Poll_Button_Latched = False Or Pic_Kit_Error = False) Then
                PKSA_Count = PICkitS.Device.How_Many_PICkitSerials_Are_Attached()
                If PKSA_Count = 0 Then
                    Hardware_Detected = 0
                    ToolStripStatusLabel1.Text = " PKSA Hardware Not Connected"
                    ToolStripStatusLabel1.BackColor = Color.Red
                    TabControl1.Enabled = False
                    PICkitConnect.Enabled = False
                    Poll.Enabled = False
                    ReadOnce.Enabled = False
                    Poll_Button_Latched = False
                    Poll.BackColor = Color.Transparent
                End If
            End If
        End If
    End Sub
#End Region

#Region "Read "
    Private Sub Test_Click(sender As Object, e As EventArgs) Handles Test.Click
        If Test_Button_Latched = False Then
            Test_Button_Latched = True
            Test.BackColor = Color.GreenYellow
        Else
            Test_Button_Latched = False
            Test.BackColor = Color.Transparent
        End If
        'Append_Text1("")
        RichTextBox1.Text = ""
    End Sub

    Private Sub Button14_Click_1(sender As Object, e As EventArgs)
        Init_PKSA(0)
    End Sub

    Private Sub ReadOnce_Click_1(sender As Object, e As EventArgs) Handles ReadOnce.Click
        If Pic_Kit_Error = False Then
            Read_All(0)
        End If
    End Sub

    Private Sub Poll_Click_1(sender As Object, e As EventArgs) Handles Poll.Click
        If Poll_Button_Latched = False Then
            Poll_Button_Latched = True
            Poll.BackColor = Color.GreenYellow
        Else
            Poll_Button_Latched = False
            Poll.BackColor = Color.Red
        End If
    End Sub

#End Region

    Private Sub Button1_Click_1(sender As Object, e As EventArgs)

        Write_Buf(1) = 0
        Write_Word(&H3B)

    End Sub


#Region "MFR Tab"

    Private Sub Button_MFR_Read_Click(sender As Object, e As EventArgs) Handles Button_MFR_Read.Click
        Append_Text1("Started to Read PMBus MFR Data........" & vbCrLf)
        Update_Pmbus_MFR(0)
    End Sub




    Private Sub MFR_EN_Click_1(sender As Object, e As EventArgs)
#If remove Then
        Dim Addr_cmd_code As Byte = Convert.ToByte(TextBox10.Text, 16)
        Dim Data0 As Byte = Convert.ToByte(TextBox19.Text Mod 256, 16)
        Dim Data1 As Byte = Convert.ToByte(TextBox19.Text \ 256, 16)
#End If
        Write_Buf(0) = 6 'Convert.ToByte(TextBox32.Text, 10) ' Count
        Write_Buf(1) = 0 'Convert.ToByte(NumericUpDown15.Value, 10) 'Page
        Write_Buf(2) = &HD1 'Addr_cmd_code      'Command Code

        Write_Buf(3) = &H4C 'Data LSB
        Write_Buf(4) = &H69 'Data MSB

        Write_Buf(5) = &H6F 'Data LSB
        Write_Buf(6) = &H6E 'Data MSB

        Write_Block(&HD1)

    End Sub
#End Region

#Region "Constant"
    Private Sub Button13_Click_1(sender As Object, e As EventArgs) Handles Button13.Click
        Append_Text1("Started to Read PMBus Constant Data........" & vbCrLf)
        Update_Pmbus_Constant_1(0)
    End Sub
#End Region


    Private Sub Button_LogRead_Click(sender As Object, e As EventArgs) Handles Button_LogRead.Click
        Append_Text1("Started to Read PMBus Event Log Data........" & vbCrLf)
        Update_Pmbus_Log(0)
    End Sub





    Private Sub Button14_Click_2(sender As Object, e As EventArgs)
        Write_Buf(0) = Convert.ToByte(NumericUpDown_SET_Vbulk.Value, 10)
        Write_Buf(1) = 0
        Write_Word(&HD0)
    End Sub


#Region "Status"

#If remove Then
    Private Sub Update_Vout()
        Dim str As String = Read_Word(&H7A) 'DataGridView1.Rows(3).Cells(3).Value()
        Dim Bits As Byte
        Bits = Convert.ToUInt16(str, 16)

        If (Bits And &H80) = &H80 Then
            PictureBox24.BackColor = Color.Red
        Else
            PictureBox24.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox22.BackColor = Color.Red
        Else
            PictureBox22.BackColor = Color.Gainsboro
        End If

        If (Bits And &H20) Then
            PictureBox20.BackColor = Color.Red
        Else
            PictureBox20.BackColor = Color.Gainsboro
        End If

        If (Bits And &H10) = &H10 Then
            PictureBox18.BackColor = Color.Red
        Else
            PictureBox18.BackColor = Color.Gainsboro
        End If

        If (Bits And &H8) = &H8 Then
            PictureBox23.BackColor = Color.Red
        Else
            PictureBox23.BackColor = Color.Gainsboro
        End If

        If (Bits And &H4) = &H4 Then
            PictureBox19.BackColor = Color.Red
        Else
            PictureBox19.BackColor = Color.Gainsboro
        End If

        If (Bits And &H2) = &H2 Then
            PictureBox21.BackColor = Color.Red
        Else
            PictureBox21.BackColor = Color.Gainsboro
        End If

        If (Bits And &H1) = &H1 Then
            PictureBox17.BackColor = Color.Red
        Else
            PictureBox17.BackColor = Color.Gainsboro
        End If
#If 1 Then

        'Status VOUT

        If (Bits And &H80) = &H80 Then
            PictureBox89.BackColor = Color.Red
        Else
            PictureBox89.BackColor = Color.Gainsboro
        End If
#If 0 Then
            If (Bits And &H40) = &H40 Then
                PictureBox97.BackColor = Color.Red
            Else
                PictureBox97.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox20.BackColor = Color.Red
            Else
                PictureBox20.BackColor = Color.Gainsboro
            End If
#End If
        If (Bits And &H10) = &H10 Then
            PictureBox129.BackColor = Color.Red
        Else
            PictureBox129.BackColor = Color.Gainsboro
        End If
#If 0 Then

            If (Bits And &H8) = &H8 Then
                PictureBox23.BackColor = Color.Red
            Else
                PictureBox23.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox19.BackColor = Color.Red
            Else
                PictureBox19.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox21.BackColor = Color.Red
            Else
                PictureBox21.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox17.BackColor = Color.Red
            Else
                PictureBox17.BackColor = Color.Gainsboro
            End If
#End If

#End If

    End Sub

    Private Sub update_Iout()
        Dim str As String = Read_Word(&H7B) 'DataGridView1.Rows(3).Cells(3).Value()
        Dim Bits As Byte
        Bits = Convert.ToUInt16(str, 16)
        'Status IOUT
        ' Bits = Convert.ToUInt16(DataGridView1.Rows(4).Cells(3).Value(), 16)

        If (Bits And &H80) = &H80 Then
            PictureBox32.BackColor = Color.Red
        Else
            PictureBox32.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox30.BackColor = Color.Red
        Else
            PictureBox30.BackColor = Color.Gainsboro
        End If

        If (Bits And &H20) Then
            PictureBox28.BackColor = Color.Red
        Else
            PictureBox28.BackColor = Color.Gainsboro
        End If

        If (Bits And &H10) = &H10 Then
            PictureBox26.BackColor = Color.Red
        Else
            PictureBox26.BackColor = Color.Gainsboro
        End If

        If (Bits And &H8) = &H8 Then
            PictureBox31.BackColor = Color.Red
        Else
            PictureBox31.BackColor = Color.Gainsboro
        End If

        If (Bits And &H4) = &H4 Then
            PictureBox27.BackColor = Color.Red
        Else
            PictureBox27.BackColor = Color.Gainsboro
        End If

        If (Bits And &H2) = &H2 Then
            PictureBox29.BackColor = Color.Red
        Else
            PictureBox29.BackColor = Color.Gainsboro
        End If

        If (Bits And &H1) = &H1 Then
            PictureBox25.BackColor = Color.Red
        Else
            PictureBox25.BackColor = Color.Gainsboro
        End If


#If 1 Then
        If (Bits And &H80) = &H80 Then
            PictureBox81.BackColor = Color.Red
        Else
            PictureBox81.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox92.BackColor = Color.Red
        Else
            PictureBox92.BackColor = Color.Gainsboro
        End If

        If (Bits And &H20) Then
            PictureBox108.BackColor = Color.Red
        Else
            PictureBox108.BackColor = Color.Gainsboro
        End If
#If 0 Then

            If (Bits And &H10) = &H10 Then
                PictureBox26.BackColor = Color.Red
            Else
                PictureBox26.BackColor = Color.Gainsboro
            End If
#End If

        If (Bits And &H8) = &H8 Then
            PictureBox84.BackColor = Color.Red
        Else
            PictureBox84.BackColor = Color.Gainsboro
        End If
#If 0 Then

            If (Bits And &H4) = &H4 Then
                PictureBox27.BackColor = Color.Red
            Else
                PictureBox27.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox29.BackColor = Color.Red
            Else
                PictureBox29.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox25.BackColor = Color.Red
            Else
                PictureBox25.BackColor = Color.Gainsboro
            End If
#End If

#End If
    End Sub

    Private Sub update_Input()
        Dim str As String = Read_Word(&H7C) 'DataGridView1.Rows(3).Cells(3).Value()
        Dim Bits As Byte
        Bits = Convert.ToUInt16(str, 16)

        If Page_sel = 0 Then

            'Status Input
            Bits = Convert.ToUInt16(DataGridView1.Rows(5).Cells(3).Value(), 16)

            If (Bits And &H80) = &H80 Then
                PictureBox56.BackColor = Color.Red
            Else
                PictureBox56.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox48.BackColor = Color.Red
            Else
                PictureBox48.BackColor = Color.Gainsboro
            End If

            If (Bits And &H20) Then
                PictureBox40.BackColor = Color.Red
            Else
                PictureBox40.BackColor = Color.Gainsboro
            End If

            If (Bits And &H10) = &H10 Then
                PictureBox34.BackColor = Color.Red
            Else
                PictureBox34.BackColor = Color.Gainsboro
            End If

            If (Bits And &H8) = &H8 Then
                PictureBox52.BackColor = Color.Red
            Else
                PictureBox52.BackColor = Color.Gainsboro
            End If

            If (Bits And &H4) = &H4 Then
                PictureBox36.BackColor = Color.Red
            Else
                PictureBox36.BackColor = Color.Gainsboro
            End If

            If (Bits And &H2) = &H2 Then
                PictureBox44.BackColor = Color.Red
            Else
                PictureBox44.BackColor = Color.Gainsboro
            End If

            If (Bits And &H1) = &H1 Then
                PictureBox33.BackColor = Color.Red
            Else
                PictureBox33.BackColor = Color.Gainsboro
            End If


#If 1 Then

            If (Bits And &H80) = &H80 Then
                PictureBox96.BackColor = Color.Red
            Else
                PictureBox96.BackColor = Color.Gainsboro
            End If

            If (Bits And &H40) = &H40 Then
                PictureBox112.BackColor = Color.Red
            Else
                PictureBox112.BackColor = Color.Gainsboro
            End If
#If 0 Then

                If (Bits And &H20) Then
                    PictureBox40.BackColor = Color.Red
                Else
                    PictureBox40.BackColor = Color.Gainsboro
                End If
#End If
            If (Bits And &H10) = &H10 Then
                PictureBox140.BackColor = Color.Red
            Else
                PictureBox140.BackColor = Color.Gainsboro
            End If
#If 0 Then
                If (Bits And &H8) = &H8 Then
                    PictureBox52.BackColor = Color.Red
                Else
                    PictureBox52.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox36.BackColor = Color.Red
                Else
                    PictureBox36.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox44.BackColor = Color.Red
                Else
                    PictureBox44.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox33.BackColor = Color.Red
                Else
                    PictureBox33.BackColor = Color.Gainsboro
                End If
#End If

        End If
#End If
    End Sub
    Private Sub Update_temperature()
        Dim str As String = Read_Word(&H7D) 'DataGridView1.Rows(3).Cells(3).Value()
        Dim Bits As Byte
        Bits = Convert.ToUInt16(str, 16)
        'Status Temperature
        'Bits = Convert.ToUInt16(DataGridView1.Rows(6).Cells(3).Value(), 16)

        If (Bits And &H80) = &H80 Then
            PictureBox60.BackColor = Color.Red
        Else
            PictureBox60.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox54.BackColor = Color.Red
        Else
            PictureBox54.BackColor = Color.Gainsboro
        End If

        If (Bits And &H20) Then
            PictureBox46.BackColor = Color.Red
        Else
            PictureBox46.BackColor = Color.Gainsboro
        End If

        If (Bits And &H10) = &H10 Then
            PictureBox38.BackColor = Color.Red
        Else
            PictureBox38.BackColor = Color.Gainsboro
        End If

        If (Bits And &H8) = &H8 Then
            PictureBox58.BackColor = Color.Red
        Else
            PictureBox58.BackColor = Color.Gainsboro
        End If

        If (Bits And &H4) = &H4 Then
            PictureBox42.BackColor = Color.Red
        Else
            PictureBox42.BackColor = Color.Gainsboro
        End If

        If (Bits And &H2) = &H2 Then
            PictureBox50.BackColor = Color.Red
        Else
            PictureBox50.BackColor = Color.Gainsboro
        End If

        If (Bits And &H1) = &H1 Then
            PictureBox35.BackColor = Color.Red
        Else
            PictureBox35.BackColor = Color.Gainsboro
        End If

#If 1 Then
        If (Bits And &H80) = &H80 Then
            PictureBox88.BackColor = Color.Red
        Else
            PictureBox88.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox99.BackColor = Color.Red
        Else
            PictureBox99.BackColor = Color.Gainsboro
        End If
#If 0 Then
                If (Bits And &H20) Then
                    PictureBox46.BackColor = Color.Red
                Else
                    PictureBox46.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox38.BackColor = Color.Red
                Else
                    PictureBox38.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox58.BackColor = Color.Red
                Else
                    PictureBox58.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox42.BackColor = Color.Red
                Else
                    PictureBox42.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox50.BackColor = Color.Red
                Else
                    PictureBox50.BackColor = Color.Gainsboro
                End If

                If (Bits And &H1) = &H1 Then
                    PictureBox35.BackColor = Color.Red
                Else
                    PictureBox35.BackColor = Color.Gainsboro
                End If
#End If
#End If
    End Sub

    Private Sub update_CML()
        Dim str As String = Read_Word(&H7E) 'DataGridView1.Rows(3).Cells(3).Value()
        Dim Bits As Byte
        Bits = Convert.ToUInt16(str, 16)

        'Status CML
        Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(7).Cells(3).Value(), 16)

        If (Bits And &H80) = &H80 Then
            PictureBox62.BackColor = Color.Red
        Else
            PictureBox62.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox57.BackColor = Color.Red
        Else
            PictureBox57.BackColor = Color.Gainsboro
        End If

        If (Bits And &H20) Then
            PictureBox49.BackColor = Color.Red
        Else
            PictureBox49.BackColor = Color.Gainsboro
        End If

        If (Bits And &H10) = &H10 Then
            PictureBox41.BackColor = Color.Red
        Else
            PictureBox41.BackColor = Color.Gainsboro
        End If

        If (Bits And &H8) = &H8 Then
            PictureBox61.BackColor = Color.Red
        Else
            PictureBox61.BackColor = Color.Gainsboro
        End If

        If (Bits And &H4) = &H4 Then
            PictureBox45.BackColor = Color.Red
        Else
            PictureBox45.BackColor = Color.Gainsboro
        End If

        If (Bits And &H2) = &H2 Then
            PictureBox53.BackColor = Color.Red
        Else
            PictureBox53.BackColor = Color.Gainsboro
        End If

        If (Bits And &H1) = &H1 Then
            PictureBox37.BackColor = Color.Red
        Else
            PictureBox37.BackColor = Color.Gainsboro
        End If

    End Sub

#End If
#If Remove Then
    Private Sub Update_Fan()
        Dim str As String = Read_Word(&H81) 'DataGridView1.Rows(3).Cells(3).Value()
        Dim Bits As Byte
        Bits = Convert.ToUInt16(str, 16)
        'Status Fan 1 & 2
        Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(10).Cells(3).Value(), 16)

        If (Bits And &H80) = &H80 Then
            PictureBox168.BackColor = Color.Red
        Else
            PictureBox168.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox166.BackColor = Color.Red
        Else
            PictureBox166.BackColor = Color.Gainsboro
        End If

        If (Bits And &H20) Then
            PictureBox164.BackColor = Color.Red
        Else
            PictureBox164.BackColor = Color.Gainsboro
        End If

        If (Bits And &H10) = &H10 Then
            PictureBox162.BackColor = Color.Red
        Else
            PictureBox162.BackColor = Color.Gainsboro
        End If

        If (Bits And &H8) = &H8 Then
            PictureBox167.BackColor = Color.Red
        Else
            PictureBox167.BackColor = Color.Gainsboro
        End If

        If (Bits And &H4) = &H4 Then
            PictureBox163.BackColor = Color.Red
        Else
            PictureBox163.BackColor = Color.Gainsboro
        End If

        If (Bits And &H2) = &H2 Then
            PictureBox165.BackColor = Color.Red
        Else
            PictureBox165.BackColor = Color.Gainsboro
        End If

        If (Bits And &H1) = &H1 Then
            PictureBox161.BackColor = Color.Red
        Else
            PictureBox161.BackColor = Color.Gainsboro
        End If
    End Sub
    Private Sub Update_Others()
        Dim str As String = Read_Word(&H7F) 'DataGridView1.Rows(3).Cells(3).Value()
        Dim Bits As Byte
        Bits = Convert.ToUInt16(str, 16)
        Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(8).Cells(3).Value(), 16)
#If 0 Then
                If (Bits And &H80) = &H80 Then
                    PictureBox39.BackColor = Color.Red
                Else
                    PictureBox39.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox47.BackColor = Color.Red
                Else
                    PictureBox47.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox55.BackColor = Color.Red
                Else
                    PictureBox55.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox63.BackColor = Color.Red
                Else
                    PictureBox63.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox43.BackColor = Color.Red
                Else
                    PictureBox43.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox59.BackColor = Color.Red
                Else
                    PictureBox59.BackColor = Color.Gainsboro
                End If
#End If
        If (Bits And &H2) = &H2 Then
            PictureBox76.BackColor = Color.Red
        Else
            PictureBox76.BackColor = Color.Gainsboro
        End If

        If (Bits And &H1) = &H1 Then
            PictureBox80.BackColor = Color.Red
        Else
            PictureBox80.BackColor = Color.Gainsboro
        End If

    End Sub


    Private Sub update_Mfr()

        'Status MFR
        Dim str As String = Read_Word(&H80) 'DataGridView1.Rows(3).Cells(3).Value()
        Dim Bits As Byte
        Bits = Convert.ToUInt16(str, 16)
        ' Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(9).Cells(3).Value(), 16)

        If (Bits And &H80) = &H80 Then
            PictureBox169.BackColor = Color.Red
        Else
            PictureBox169.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox171.BackColor = Color.Red
        Else
            PictureBox171.BackColor = Color.Gainsboro
        End If

        If (Bits And &H20) Then
            PictureBox173.BackColor = Color.Red
        Else
            PictureBox173.BackColor = Color.Gainsboro
        End If

        If (Bits And &H10) = &H10 Then
            PictureBox175.BackColor = Color.Red
        Else
            PictureBox175.BackColor = Color.Gainsboro
        End If

        If (Bits And &H8) = &H8 Then
            PictureBox170.BackColor = Color.Red
        Else
            PictureBox170.BackColor = Color.Gainsboro
        End If

        If (Bits And &H4) = &H4 Then
            PictureBox174.BackColor = Color.Red
        Else
            PictureBox174.BackColor = Color.Gainsboro
        End If

        If (Bits And &H2) = &H2 Then
            PictureBox172.BackColor = Color.Red
        Else
            PictureBox172.BackColor = Color.Gainsboro
        End If

        If (Bits And &H1) = &H1 Then
            PictureBox176.BackColor = Color.Red
        Else
            PictureBox176.BackColor = Color.Gainsboro
        End If

#If 1 Then
        Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(8).Cells(3).Value(), 16)
#If 1 Then

        If (Bits And &H80) = &H80 Then
            PictureBox39.BackColor = Color.Red
        Else
            PictureBox39.BackColor = Color.Gainsboro
        End If

        If (Bits And &H40) = &H40 Then
            PictureBox47.BackColor = Color.Red
        Else
            PictureBox47.BackColor = Color.Gainsboro
        End If

        If (Bits And &H20) Then
            PictureBox55.BackColor = Color.Red
        Else
            PictureBox55.BackColor = Color.Gainsboro
        End If

        If (Bits And &H10) = &H10 Then
            PictureBox63.BackColor = Color.Red
        Else
            PictureBox63.BackColor = Color.Gainsboro
        End If

        If (Bits And &H8) = &H8 Then
            PictureBox43.BackColor = Color.Red
        Else
            PictureBox43.BackColor = Color.Gainsboro
        End If

        If (Bits And &H4) = &H4 Then
            PictureBox59.BackColor = Color.Red
        Else
            PictureBox59.BackColor = Color.Gainsboro
        End If

        If (Bits And &H2) = &H2 Then
            PictureBox51.BackColor = Color.Red
        Else
            PictureBox51.BackColor = Color.Gainsboro
        End If
#End If
        If (Bits And &H1) = &H1 Then
            PictureBox72.BackColor = Color.Red
        Else
            PictureBox72.BackColor = Color.Gainsboro
        End If

        'Status MFR
        Bits = 0 'Convert.ToUInt16(DataGridView1.Rows(9).Cells(3).Value(), 16)
#If 0 Then

                If (Bits And &H80) = &H80 Then
                    PictureBox169.BackColor = Color.Red
                Else
                    PictureBox169.BackColor = Color.Gainsboro
                End If

                If (Bits And &H40) = &H40 Then
                    PictureBox171.BackColor = Color.Red
                Else
                    PictureBox171.BackColor = Color.Gainsboro
                End If

                If (Bits And &H20) Then
                    PictureBox173.BackColor = Color.Red
                Else
                    PictureBox173.BackColor = Color.Gainsboro
                End If

                If (Bits And &H10) = &H10 Then
                    PictureBox175.BackColor = Color.Red
                Else
                    PictureBox175.BackColor = Color.Gainsboro
                End If

                If (Bits And &H8) = &H8 Then
                    PictureBox170.BackColor = Color.Red
                Else
                    PictureBox170.BackColor = Color.Gainsboro
                End If

                If (Bits And &H4) = &H4 Then
                    PictureBox174.BackColor = Color.Red
                Else
                    PictureBox174.BackColor = Color.Gainsboro
                End If

                If (Bits And &H2) = &H2 Then
                    PictureBox172.BackColor = Color.Red
                Else
                    PictureBox172.BackColor = Color.Gainsboro
                End If
#End If
        If (Bits And &H1) = &H1 Then
            PictureBox72.BackColor = Color.Red
        Else
            PictureBox72.BackColor = Color.Gainsboro
        End If
#End If
    End Sub
#End If
    Private Sub Button34_Click(sender As Object, e As EventArgs) Handles Button_status_write.Click 'write status
        If TextBox_status_Data.Text = "" Or TextBox_status_Data.TextLength < 1 Then
            MsgBox("Please  Input your data to write !")

        Else
            Dim Data0 As Byte = Convert.ToByte(TextBox_status_Data.Text Mod 256, 16)

            Write_Buf(0) = 4 ' Count
            Write_Buf(1) = NumericUpDown_page.Value 'Page
            Write_Buf(2) = &H1B ''SMB Alert Mask Command

            If ListBox_Cmd.SelectedIndex = 0 Then
                Write_Buf(3) = &H78
            ElseIf ListBox_Cmd.SelectedIndex = 1 Then
                Write_Buf(3) = &H79
            ElseIf ListBox_Cmd.SelectedIndex = 2 Then
                Write_Buf(3) = &H7A
            ElseIf ListBox_Cmd.SelectedIndex = 3 Then
                Write_Buf(3) = &H7B
            ElseIf ListBox_Cmd.SelectedIndex = 4 Then
                Write_Buf(3) = &H7C
            ElseIf ListBox_Cmd.SelectedIndex = 5 Then
                Write_Buf(3) = &H7D
            ElseIf ListBox_Cmd.SelectedIndex = 6 Then
                Write_Buf(3) = &H7E
            ElseIf ListBox_Cmd.SelectedIndex = 7 Then
                Write_Buf(3) = &H7F
            ElseIf ListBox_Cmd.SelectedIndex = 8 Then
                Write_Buf(3) = &H80
            Else
                MsgBox("Please Select Your Command!")
            End If
            '   Write_Buf(3) = &H7A      'Status Vout Command

            Write_Buf(4) = Data0 'Mask Value
            Write_Block(&H5)


        End If
    End Sub

    Private Sub update_pic()


    End Sub
    Private Sub Button_status_read_Click(sender As Object, e As EventArgs) Handles Button_status_read.Click
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0
        Dim Cmd_SMB_Alert As Byte = &H1B

        Slave_Addr = NumericUpDown_S_Addr.Value

        Write_Buf(0) = 3 ' Count
        Write_Buf(1) = NumericUpDown_page.Value ' Status Vout Command
        Write_Buf(2) = Cmd_SMB_Alert ' SMB Alert Command 

        If ListBox_Cmd.SelectedIndex = 0 Then
            Write_Buf(3) = &H78
        ElseIf ListBox_Cmd.SelectedIndex = 1 Then
            Write_Buf(3) = &H79
        ElseIf ListBox_Cmd.SelectedIndex = 2 Then
            Write_Buf(3) = &H7A
        ElseIf ListBox_Cmd.SelectedIndex = 3 Then
            Write_Buf(3) = &H7B
        ElseIf ListBox_Cmd.SelectedIndex = 4 Then
            Write_Buf(3) = &H7C
        ElseIf ListBox_Cmd.SelectedIndex = 5 Then
            Write_Buf(3) = &H7D
        ElseIf ListBox_Cmd.SelectedIndex = 6 Then
            Write_Buf(3) = &H7E
        ElseIf ListBox_Cmd.SelectedIndex = 7 Then
            Write_Buf(3) = &H7F
        ElseIf ListBox_Cmd.SelectedIndex = 8 Then
            Write_Buf(3) = &H80
        Else
            MsgBox("Please select  your Command!")
        End If

        ' Write_Buf(3) = &H7A ' Status Vout Command

        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(&H1B, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(1), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(2), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(3), CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr + 1, CRC8_Byte)

        If (PICkitS.I2CM.Write(Slave_Addr, &H6, 4, Write_Buf, Return_Str)) Then

            Thread.Sleep(1)

            Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Return_Str & " "

            Return_Str = Convert.ToString(Write_Buf(1), 16).ToUpper
            If Not Return_Str.Length = 2 Then
                Return_Str = "0" & Return_Str
            End If
            Write_Buf_Str = Write_Buf_Str & Return_Str & " "

            Append_Text1("Write Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Write_Buf_Str & vbCrLf)

            If (PICkitS.I2CM.Receive(Slave_Addr + 1, 3, Read_Buf, Return_Str)) Then

                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(0), CRC8_Byte)
                CRC8_Byte = PICkitS.Utilities.calculate_crc8(Read_Buf(1), CRC8_Byte)

                Read_Buf_Str = " "
                Return_Str = Convert.ToString(Read_Buf(0), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                Return_Str = Convert.ToString(Read_Buf(1), 16).ToUpper
                If Not Return_Str.Length = 2 Then
                    Return_Str = "0" & Return_Str
                End If
                Read_Buf_Str = Read_Buf_Str & Return_Str & " "

                If CRC8_Byte = Read_Buf(2) Then
                    SMB_Mask = Read_Buf(1)
                    TextBox_Status_Data_r.Text = Convert.ToString(SMB_Mask, 10).ToUpper
                    Append_Text1("Read Block Sucessful - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                Else
                    Append_Text1("Read Block PEC Error - " & Convert.ToString(&H6, 16).ToUpper & "- " & Read_Buf_Str & "- CRC8 -" & Convert.ToString(CRC8_Byte, 16).ToUpper & vbCrLf)
                End If

            Else
                Append_Text1("Error Reading Data From the Device" & vbCrLf)
                Pic_Kit_Error = True
            End If
        Else
            Append_Text1("Error Writing Data to the Device" & vbCrLf)
            Pic_Kit_Error = True
        End If
    End Sub

    Private Sub Button_Clear_Faults_Click_1(sender As Object, e As EventArgs) Handles Button_Clear_Faults.Click
        Send_Byte(&H3)
    End Sub
#End Region
#Region "Control Tab"



    Private Sub Button_SET_Duty_Click(sender As Object, e As EventArgs) Handles Button_SET_Duty.Click
        Dim Cmd_SET_Duty As Byte
        Cmd_SET_Duty = &H3B
        Write_Buf(0) = Convert.ToByte(NumericUpDown_SET_Duty.Value, 10)
        ' Write_Buf(1) = 0
        Write_Byte(Cmd_SET_Duty)
    End Sub

    Private Sub Button_GET_Duty_Click(sender As Object, e As EventArgs) Handles Button_GET_Duty.Click
        'TextBox_GET_Duty.Text = Convert.ToString(Read_Byte(&H3B))

        Dim str As String
        str = Mid(Read_Byte(&H3B), 1, 2)
        TextBox_GET_Duty.Text = Convert.ToString(Convert.ToByte(str, &H10))
    End Sub


    Private Sub Button_GET_Vbulk_Click(sender As Object, e As EventArgs) Handles Button_GET_Vbulk.Click
        Dim str As String
        Dim str1 As String
        Dim str2 As String
        Dim i As UInt16 = 0
        Dim j As UInt16 = 0
        str = Read_Word(&H56)
        str1 = Mid(str, 1, 2)
        i = Convert.ToByte(str1, &H10)
        str2 = Mid(str, 4, 2)
        j = Convert.ToByte(str2, &H10)


        TextBox_GET_Vbulk.Text = Convert.ToString(i + 256 - 400)
    End Sub

    Private Sub Button_SET_Vbulk_Click(sender As Object, e As EventArgs) Handles Button_SET_Vbulk.Click
        Dim Cmd_SET_Vbulk As Byte

        Dim i As Byte = 0
        Dim j As UInt16 = 0

        Cmd_SET_Vbulk = &H56
        j = NumericUpDown_SET_Vbulk.Value + 400
        ' i = j And (&HFF)
        Write_Buf(0) = Convert.ToByte((j Mod 256), 10)
        i = Convert.ToByte(j / 256)
        Write_Buf(1) = Convert.ToByte((i)) ', 10)
        'Write_Buf(0) = Convert.ToByte((NumericUpDown_SET_Vbulk.Value + 400) Mod 256, 10)
        ' Write_Buf(1) = Convert.ToByte((NumericUpDown_SET_Vbulk.Value + 400) / 256, 10)
        'Write_Byte(Cmd_SET_Vbulk)
        Write_Word(Cmd_SET_Vbulk)
    End Sub


    Private Sub Button_SET_IOUT_OC_Fault_Click(sender As Object, e As EventArgs) Handles Button_SET_IOUT_OC_Fault.Click
        Dim SET_IOUT_OC_Fault As Byte
        SET_IOUT_OC_Fault = &H46
        Write_Buf(0) = Convert.ToByte((NumericUpDown_SET_IOUT_F.Value)) ', 10)
        Write_Byte(SET_IOUT_OC_Fault)
    End Sub

    Private Sub Button_GET_IOUT_OC_Fault_Limit_Click(sender As Object, e As EventArgs) Handles Button_GET_IOUT_OC_Fault_Limit.Click
        Dim str As String
        str = Mid(Read_Byte(&H46), 1, 2)
        'Dim k As Byte
        'k = Convert.ToByte(str, &H10)
        TextBox_GET_OC_Fault_Limit.Text = Convert.ToString(Convert.ToByte(str, &H10))

    End Sub

    Private Sub Button_SET_IOUT_OC_WARN_Click(sender As Object, e As EventArgs) Handles Button_SET_IOUT_OC_WARN.Click
        Dim Cmd_SET_Vbulk As Byte
        Cmd_SET_Vbulk = &H4A
        Write_Buf(0) = Convert.ToByte((NumericUpDown_Set_Warn.Value), 10)
        Write_Byte(Cmd_SET_Vbulk)
    End Sub

    Private Sub Button_IOUT_OC_Warn_Click(sender As Object, e As EventArgs) Handles Button_IOUT_OC_Warn.Click
        'Dim i As Decimal
        Dim str As String
        str = Mid(Read_Byte(&H4A), 1, 2)
        ' Dim j As Decimal
        Dim k As Byte
        k = Convert.ToByte(str, &H10)
        ' j = Convert.ToDecimal(k)
        TextBox_Set_Warn.Text = Convert.ToString(k) 'Convert.ToString(Read_Byte(&H4A))

        'i = Convert.ToDecimal(Read_Byte(&H4A))
        'TextBox_Set_Warn.Text = Convert.ToString(i)
    End Sub

    Private Sub RadioButton_Disable_CheckedChanged(sender As Object, e As EventArgs) 'Handles RadioButton_Disable.CheckedChanged

        RadioButton_Enable.Checked = False
        RadioButton_Disable.Checked = True


        ' Write_Buf(0) = &H56
        ' Write_Byte(&HEA)

    End Sub

    Private Sub Button7_Click_1(sender As Object, e As EventArgs) Handles Button_PSUOn.Click
        'PSU ON
        Write_Buf(0) = &H0
        Write_Byte(&H1)
    End Sub

    Private Sub Button_PSUOff_Click(sender As Object, e As EventArgs) Handles Button_PSUOff.Click
        'PSU OFF
        Write_Buf(0) = &H80
        Write_Byte(&H1)
    End Sub

    Private Sub RadioButton_Enable_CheckedChanged(sender As Object, e As EventArgs) ' Handles RadioButton_Enable.CheckedChanged

        '  Write_Buf(0) = &H96
        ' Write_Byte(&HEA)
        RadioButton_Enable.Checked = True
        RadioButton_Disable.Checked = False

    End Sub

    Private Sub CheckBox5_CheckedChanged(sender As Object, e As EventArgs) Handles CheckBox5.CheckedChanged

#If 0 Then

        If CheckBox5.Checked = False Then
            CheckBox5.Checked = True


        Else
            CheckBox5.Checked = False

        End If
#End If
    End Sub

    Private Sub CheckBox1_CheckedChanged_1(sender As Object, e As EventArgs)

    End Sub



    Private Sub Button_PSU_ON_Click(sender As Object, e As EventArgs)
        'PSU ON
        Write_Buf(0) = &H0
        Write_Byte(&H1)
    End Sub

    Private Sub Button_PSU_OFF_Click(sender As Object, e As EventArgs)
        'PSU OFF
        Write_Buf(0) = &H80
        Write_Byte(&H1)
    End Sub

    Private Sub Button21_Click(sender As Object, e As EventArgs)
        Write_Buf(0) = Convert.ToByte(NumericUpDown_SET_Duty.Value, 10)
        Write_Buf(1) = 0
        Write_Word(&H3B)
    End Sub

    Private Sub Button18_Click(sender As Object, e As EventArgs)
        'Read Fan Duty
        Dim str As String = Read_Word(&H3B)
        TextBox_GET_Duty.Text = Pmb_Hex_Data
    End Sub

    Private Sub Button7_Click(sender As Object, e As EventArgs)
        'Read Vbulk
        Dim str As String = Read_Word(&HD0)
        TextBox_GET_Vbulk.Text = Pmb_Hex_Data
    End Sub

#End Region
#Region "Calibration Tab"

#Region "cips conmands"

    Public Sub Get_Index()
        If RadioButton_p.Checked = True Then
            RadioButton_s.Checked = False

            Select Case ListBox_index.SelectedIndex
                Case 0
                    index(0) = &H10
                    ' index(2) = 1
                    ListBox_adc.SelectedIndex = 0
                Case 1, 2
                    index(0) = &H20
                    index(1) = ListBox_index.SelectedIndex - 1
                    ListBox_adc.SelectedIndex = 1
                Case 3, 4
                    index(0) = &H21
                    index(1) = ListBox_index.SelectedIndex - 3
                    ListBox_adc.SelectedIndex = 1
                Case 5, 6
                    index(0) = &H30
                    index(1) = ListBox_index.SelectedIndex - 5
                    ListBox_adc.SelectedIndex = 2

                Case 7, 8
                    index(0) = &H31
                    index(1) = ListBox_index.SelectedIndex - 7
                    ListBox_adc.SelectedIndex = 2
            End Select

        ElseIf RadioButton_s.Checked = True Then
            RadioButton_p.Checked = False

            Select Case ListBox_index.SelectedIndex
                Case 0
                    index(0) = &H40
                    index(1) = 0
                    ListBox_adc.SelectedIndex = 0
                Case 1, 2
                    index(0) = &H50
                    index(1) = ListBox_index.SelectedIndex - 1
                    ListBox_adc.SelectedIndex = 2
                Case 3
                    index(0) = &H60
                    index(1) = 0
                    ListBox_adc.SelectedIndex = 5
                Case 4, 5
                    index(0) = &H70
                    index(1) = ListBox_index.SelectedIndex - 4
                    ListBox_adc.SelectedIndex = 7

                Case 6, 7
                    index(0) = &H90
                    index(1) = ListBox_index.SelectedIndex - 6
                    ListBox_adc.SelectedIndex = 3
                Case 8
                    index(0) = &HFE
                    index(1) = 0
            End Select
        End If

        If index(1) = 0 Then
            GroupBox_1.Text = "10%"
            GroupBox_2.Text = "20%"
            Select Case index(0)
                Case &H10, &H40, &H60
                    GroupBox_2.Text = "100%"
            End Select

        ElseIf index(1) = 1 Then
            GroupBox_1.Text = "20%"
            GroupBox_2.Text = "100%"
        End If

    End Sub

    Public Function Get_Index_adc()
        Dim index As Byte = 0

        If RadioButton_p.Checked = True Then
            index = ListBox_adc.SelectedIndex
            RadioButton_s.Checked = False

        ElseIf RadioButton_s.Checked = True Then
            RadioButton_p.Checked = False
            index = &H10 + ListBox_adc.SelectedIndex
        End If
        Return index
    End Function


    Public Function Str_format(ByVal str_v As String)
        Dim str As String = ""
        Dim i As Byte = 0
        i = Convert.ToByte(str_v)
        Select Case i
            Case 0 To 15
                str = "0" & str_v
            Case Else
                str = str_v
        End Select
        Return str
    End Function


    Public Sub Read_ADC()

        RichTextBox1.Text = ""
        Write_Buf(0) = Get_Index_adc()
        Read_Word_ADC(&HFA)
        AdcL = Read_Buf(0)
        AdcH = Read_Buf(1)

        Select Case Button_Read_Adc.Text
            Case "Read ADC"
                TextBox_Raw_ADC.Text = Str_format(Convert.ToString(Read_Buf(1))) & Str_format(Convert.ToString(Read_Buf(0))) ' MSB
                Button_Read_Adc.BackColor = Color.Red
                Button_Read_Adc.Text = "Read ADC Again"
                ADC_point1 = AdcL + 256 * AdcH
                Exit Select
            Case "Read ADC Again"

                Button_Read_Adc.Text = "Read ADC"
                TextBox_RAW_ADC_2.Text = Str_format(Convert.ToString(Read_Buf(2))) & Str_format(Convert.ToString(Read_Buf(1))) ' MSB
                ADC_point2 = AdcL + 256 * AdcH
                ThrL = AdcL 'Read_Buf(0)
                ThrH = AdcH 'Read_Buf(1)
                Button_Read_Adc.BackColor = Color.Transparent
        End Select
    End Sub


    Public Sub Primary_dispaly()
        ListBox_index.Items.Clear()

        ListBox_index.Items.Add("0x10 - Vin data (10% - 100%)")
        ListBox_index.Items.Add("0x20 - IinL data(10% -20%)")
        ListBox_index.Items.Add("0x20 - IinL data(20% -100%)")
        ListBox_index.Items.Add("0x21 - IinH data(10% -20%)")
        ListBox_index.Items.Add("0x21 - IinH data(20% -100%)")
        ListBox_index.Items.Add("0x30 - PinL data(10% -20%)")
        ListBox_index.Items.Add("0x30 - PinL data(20% -100%)")
        ListBox_index.Items.Add("0x31 - PinH data(10% -20%)")
        ListBox_index.Items.Add("0x31 - PinH data(20% -100%)")

        ListBox_adc.Items.Clear()
        ListBox_adc.Items.Add("0x00- Read Vin adc data")
        ListBox_adc.Items.Add("0x01- Read Iin adc data")
        ListBox_adc.Items.Add("0x02- Read Pin adc data")
    End Sub

    Public Sub Secondary_dispaly()
        ListBox_index.Items.Clear()
        ListBox_index.Items.Add("0x40 - Vout data (10% - 100%)")
        ListBox_index.Items.Add("0x50 - Iout data (10% -20%)")
        ListBox_index.Items.Add("0x50 - Iout data (20% -100%)")
        ListBox_index.Items.Add("0x60 - Vsb data (10% - 100%)")
        ListBox_index.Items.Add("0x70 - Isb data (10% -20%)")
        ListBox_index.Items.Add("0x70 - Isb data (20% -100%)")
        ListBox_index.Items.Add("0x90 - Ishare data (10% -20%)")
        ListBox_index.Items.Add("0x90 - Ishare data (20% -100%)")
        ListBox_index.Items.Add("0xFE - default data")

        ListBox_adc.Items.Clear()
        ListBox_adc.Items.Add("0x10- Read V1 int adc")
        ListBox_adc.Items.Add("0x11- Read V1 ext adc")
        ListBox_adc.Items.Add("0x12- Read I1 adc")
        ListBox_adc.Items.Add("0x13- Read Ishare adc")
        ListBox_adc.Items.Add("0x14- Read Ilocal adc")
        ListBox_adc.Items.Add("0x15- Read Vsb int adc")
        ListBox_adc.Items.Add("0x16- Read Vsb ext adc")
        ListBox_adc.Items.Add("0x17- Read Isb adc")
    End Sub


    Public Function get_meter_time()
        Dim A1 As Byte = 0
        Select Case (index(0))

            Case &H30, &H31
                A1 = 32
            Case Else
                A1 = 128
        End Select
        Return A1
    End Function

    Public Sub Calculate_formula()
        Dim Y2, Y1 As Integer
        Dim A As Byte
        A = get_meter_time()
        ' A = 1
        Y1 = A * Convert.ToUInt16(TextBox_input_1.Text)
        Y2 = A * Convert.ToUInt16(TextBox_input_2.Text)

        Array_c(0) = (Y2 - Y1) \ (ADC_point2 - ADC_point1)
        ' Offset = y1 - G * x1
        Array_c(1) = Y1 - Array_c(0) * ADC_point1
    End Sub

    Public Sub display_offset()
        SlopeL = Convert.ToByte(Array_c(0) Mod 256)
        SlopeH = Convert.ToByte(Array_c(0) \ 256) 'IGNORE REMAINDER
        OffsetL = Convert.ToByte(Array_c(1) Mod 256)
        OffsetH = Convert.ToByte(Array_c(1) \ 256)

        TextBox_G0.Text = Convert.ToString(SlopeL)
        TextBox_G1.Text = Convert.ToString(SlopeH)
        TextBox_offset0.Text = Convert.ToString(OffsetL)
        TextBox_offset1.Text = Convert.ToString(OffsetH)
    End Sub

#End Region

    Private Sub Button_Read_Adc_Click(sender As Object, e As EventArgs) Handles Button_Read_Adc.Click
        Read_ADC()
    End Sub

    Public Sub Calibration_Read()
        Dim Byte_Count As Byte = 6
        'Addr_w	0xC9	0x02	Index	Dataline	Addr_r	0x06	SlopeL	SlopeH OffsetL OffsetH	ThrL	ThrH
        Get_Index()
        Read_Block_calibration(&HC9, Byte_Count)

        Read_Buf(0) = Byte_Count
        SlopeL = Read_Buf(1)
        SlopeH = Read_Buf(2)
        OffsetL = Read_Buf(3)
        OffsetH = Read_Buf(4)
        ThrL = Read_Buf(5)
        ThrH = Read_Buf(6)


        Append_Text1("SlopeL-" & Convert.ToString(Read_Buf(1)) & " SlopeH-" & Convert.ToString(Read_Buf(2)) & " - " & vbCrLf)
        Append_Text1("OffsetL -" & Convert.ToString(Read_Buf(3)) & " OffsetH -" & Convert.ToString(Read_Buf(4)) & " - " & vbCrLf)
        Append_Text1("ThrL -" & Convert.ToString(Read_Buf(5)) & " ThrH -" & Convert.ToString(Read_Buf(6)) & " - " & vbCrLf)

    End Sub

    Private Sub Button_Cab_Read_Click(sender As Object, e As EventArgs) Handles Button_Cab_Read.Click
        Calibration_Read()
    End Sub

    Private Sub calibration_Write_data()
        'Addr_w	0xC9	0x08	Index	Dataline	SlopeL	SlopeH	OffsetL	OffsetH	ThrL
        Get_Index()
        Write_Block_calibration(index(0), index(1))
    End Sub
    Private Sub Button_Execute_Click(sender As Object, e As EventArgs) Handles Button_Execute.Click

        'Addr_w	0xC9	0x08	Index	Dataline	SlopeL	SlopeH	OffsetL	OffsetH	ThrL	ThrH
        calibration_Write_data()

    End Sub

    Private Sub MFR_EN_Click_2(sender As Object, e As EventArgs) Handles MFR_EN.Click
#If 0 Then

        Write_Buf(0) = &H4C
        Write_Buf(1) = &H69
        Write_Buf(2) = &H6F
        Write_Buf(2) = &H6E
        Write_DWord(&HD1)
#End If
    End Sub


    Private Sub Button_S_Enable_Click(sender As Object, e As EventArgs) Handles Button_S_Enable.Click
        'Addr_w	0xFD	0x55	0x4C
        Write_Buf(1) = &H4C
        Write_Buf(0) = &H55
        Write_Word(&HFD)

        Button_S_Enable.BackColor = Color.BlueViolet
        Button_S_Disable.BackColor = Color.Transparent
    End Sub

    Private Sub Button_S_Disable_Click(sender As Object, e As EventArgs) Handles Button_S_Disable.Click
        'Addr_w	0xFD	0xAA	0xAA
        Write_Buf(0) = &HAA
        Write_Buf(1) = &HAA
        Write_Word(&HFD)


        Button_S_Enable.BackColor = Color.Transparent
        Button_S_Disable.BackColor = Color.BlueViolet
    End Sub



    Private Sub Button_calculate_Click(sender As Object, e As EventArgs) Handles Button_calculate.Click
        Calculate_formula()
        display_offset()


    End Sub




    Private Sub RadioButton_p_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButton_p.CheckedChanged

        If RadioButton_p.Checked = True Then
            RadioButton_s.Checked = False
            Primary_dispaly()
        ElseIf RadioButton_p.Checked = False Then
            RadioButton_s.Checked = True
            Secondary_dispaly()
        End If
    End Sub

    Private Sub RadioButton_s_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButton_s.CheckedChanged

        ' ListBox_adc.SelectedIndex = 0
        If RadioButton_s.Checked = False Then
            RadioButton_p.Checked = True
            Primary_dispaly()
        ElseIf RadioButton_s.Checked = True Then
            RadioButton_p.Checked = False
            Secondary_dispaly()
        End If

        ListBox_index.SelectedIndex = 0
    End Sub


#End Region
#Region "FRU Tab"
    Dim Demo_Slave_ee_Addr As Byte = &HA8 ' eeprom_Addr_w 'cips
    Private Sub Button_FRU_File_Click(sender As Object, e As EventArgs)




#If 1 Then
        Dim FILE_NAME As String = "C:\Users\WH\Documents\test1.txt"
        Dim ByteNumber As Integer = 256 '&H100
        Dim index As Integer = 0
        If System.IO.File.Exists(FILE_NAME) = True Then

            Dim objWriter As New System.IO.StreamWriter(FILE_NAME)
            ' objWriter.Write(TextBox1.Text)
            For index = 0 To ByteNumber - 1
                objWriter.Write(Convert.ToString(Read_Buf(index)))
            Next
            objWriter.Close()
            MessageBox.Show("Text written to file")
        Else
            MessageBox.Show("File Does Not Exist")
        End If
#End If


#If 1 Then
        Dim Demo_Slave_ee_Addr As Byte = &HA9
        ' determined by Demo board firmware, &H says this is a hex number
        ' IMPORTANT - addr is not same as for Read command
        ' that command automatically increments the write addr to 
        ' obtain the read addr
        ' Dim Byte_Count As Byte = Convert.ToByte(TextBox_Receive_EE_Byte_Count.Text, 16)
        Dim Byte_Count As Byte = Convert.ToByte(TextBox_status_Data.Text, 16)
        ' assume value in textbox is hex
        Dim DataArray(256) As Byte
        ' will store returned data here
        Dim Return_Str As String = ""
        ' returns string representation of command - we won't use this
        Dim Display_Str As String = ""
        ' will display results with this
        ' Dim index As Integer
        ' used as counter
        '
        ' clear array
        For index = 0 To Byte_Count - 1
            DataArray(index) = 0
        Next
        '
        If (PICkitS.I2CM.Receive(Demo_Slave_ee_Addr, Byte_Count, DataArray, Return_Str)) Then
            ' successful, display results
            For index = 0 To Byte_Count - 1
                Display_Str += Convert.ToString(DataArray(index), 16) & " "
            Next
            RichTextBox1.Text += Display_Str & vbCrLf
        Else
            RichTextBox1.Text += "Error receiving from Demo Board EE" & vbCrLf
            PICkitS.Device.Reset_Control_Block()  ' clear any errors in PKSA
        End If

#End If

    End Sub

    Private Sub Button_Write_ee_Click(sender As Object, e As EventArgs) Handles Button_Write_ee.Click
        ' routine to write 2 byte to Demo board EE
        '&HA8
        Dim Demo_Slave_ee_Addr As Byte = &HA8 '&HA8 '&HA0
#If 1 Then
        'Dim DataArray(256) As Byte
        Dim DataArray(256) As Byte


        ' determined by Demo board firmware, &H says this is a hex number
        Dim Word_Addr As Byte = Convert.ToByte(TextBox_Write_EE_Word_Addr.Text, 16)
        ' assume value in textbox is hex
        'If TextBox_Write_EE_Byte1.TextLength IsNot 0 Then
        Dim Byte1 As Byte = Convert.ToByte(TextBox_Write_EE_Byte1.Text, 16)
        ' End If
        ' assume value in textbox is hex
        Dim Byte2 As Byte = Convert.ToByte(TextBox_Write_EE_Byte2.Text, 16)

        Dim ByteCount As Byte = 0



        ' assume value in textbox is hex
        'Dim DataArray(1) As Byte
        ' create array of length 2 bytes
        ' will store write data here - sized to fit our two bytes
        Dim Return_Str As String = ""
        ' returns string representation of command - we won't use this
        DataArray(0) = Byte1
        DataArray(1) = Byte2
        'DataArray(0) = 0 'Byte1
        'DataArray(1) = 1 'Byte2
        DataArray(3) = 2 'Byte1
        DataArray(4) = 3 'Byte2
        ' DataArray.Length = DataArray.Length Mod 255
        ' fill array
        ' If (PICkitS.I2CM.Write(Demo_Slave_ee_Addr, Word_Addr, DataArray.Length, DataArray, Return_Str)) Then
        If (PICkitS.I2CM.Write(Demo_Slave_ee_Addr, Word_Addr, DataArray.Length Mod 255, DataArray, Return_Str)) Then
            ' success
            RichTextBox1.Text += "LiteOn CIPS PMBUS:  Writing  0x" & TextBox_Write_EE_Byte1.Text & "  0x" & TextBox_Write_EE_Byte2.Text & " to EE successful" & vbCrLf
        Else
            RichTextBox1.Text += "Error writing to EE" & vbCrLf
            PICkitS.Device.Reset_Control_Block()  ' clear any errors in PKSA

            ' MessageBox.Show("Error writing to EE", "LiteOn CIPS PMBUS")
        End If
    End Sub

    Private Sub BTN_FRU_Read_Click_1(sender As Object, e As EventArgs) Handles BTN_FRU_Read.Click


        ' Dim EEPROM_Addr As Byte = Demo_Slave_ee_Addr '&HA1 '&HA5 ' A0 A2 A4 ' AddrW+1
        Dim Demo_Slave_ee_Addr As Byte = &HA8 ' &HA8 ' eeprom_Addr_w 'cips

        ' determined by Demo board firmware, &H says this is a hex number

        'Dim Word_Addr As Byte = Convert.ToByte(TextBox_Read_EE_Word_Addr.Text, 16)
        Dim Word_Addr As Byte = Convert.ToByte(TextBox_Write_EE_Word_Addr.Text, 16)

        ' assume value in textbox is hex
        Dim Byte_Count As Byte = Convert.ToByte(TextBox_Read_EE_Byte_Count.Text, 16)


        ' assume value in textbox is hex
        Dim DataArray(256) As Byte
        ' will store returned data here
        Dim Return_Str As String = ""
        ' returns string representation of command - we won't use this
        Dim Display_Str As String = ""
        ' will display results with this
        Dim index As Integer
        ' used as counter
        '
        ' clear array
        For index = 0 To Byte_Count - 1
            DataArray(index) = 0 'index
        Next
        '


        If (PICkitS.I2CM.Read(Demo_Slave_ee_Addr, Word_Addr, Byte_Count, DataArray, Return_Str)) Then
            ' successful, display results
            For index = 0 To Byte_Count - 1
                Display_Str += Convert.ToString(DataArray(index), 16) & " "
            Next
            ' RichTextBox_Display.Text += Display_Str & vbCrLf
            RichTextBox1.Text += Display_Str & vbCrLf
        Else
            ' RichTextBox_Display.Text += "Error reading Demo Board EE" & vbCrLf
            RichTextBox1.Text += "Error reading Demo Board EE" & vbCrLf
            PICkitS.Device.Reset_Control_Block()  ' clear any errors in PKSA

        End If

#If 0 Then
        If (Not File.Exists(strFile)) Then
            Try
                fs = File.Create(strFile)
                sw = File.AppendText(strFile)
                sw.WriteLine("Start Error Log for today")

            Catch ex As Exception
                MsgBox("Error Creating Log File")
            End Try
        Else
            sw = File.AppendText(strFile)
            sw.WriteLine("Error Message in  Occured at-- " & DateTime.Now)

            sw.Close()
        End If
#End If



#If 0 Then
        If (PICkitS.I2CM.Receive(Demo_Slave_ee_Addr + 1, ByteNumber, Read_Buf, Return_Str)) Then
            Label_FRU_0000.Text = Convert.ToString(Read_Buf(0))
            Label_FRU_0001.Text = Convert.ToString(Read_Buf(1))
            Label_FRU_0002.Text = Convert.ToString(Read_Buf(2))
            Label_FRU_0003.Text = Convert.ToString(Read_Buf(3))
            Label_FRU_0004.Text = Convert.ToString(Read_Buf(4))
            Label_FRU_0005.Text = Convert.ToString(Read_Buf(5))
            Label_FRU_0006.Text = Convert.ToString(Read_Buf(6))
            Label_FRU_0007.Text = Convert.ToString(Read_Buf(7))
            Label_FRU_0008.Text = Convert.ToString(Read_Buf(8))
            Label_FRU_0009.Text = Convert.ToString(Read_Buf(9))
            Label_FRU_000A.Text = Convert.ToString(Read_Buf(10))
            Label_FRU_000B.Text = Convert.ToString(Read_Buf(11))
            Label_FRU_000C.Text = Convert.ToString(Read_Buf(12))
            Label_FRU_000D.Text = Convert.ToString(Read_Buf(13))
            Label_FRU_000E.Text = Convert.ToString(Read_Buf(14))
            Label_FRU_000F.Text = Convert.ToString(Read_Buf(15))

            Label_FRU_0010.Text = Convert.ToString(Read_Buf(16))
            Label_FRU_0011.Text = Convert.ToString(Read_Buf(17))
            Label_FRU_0012.Text = Convert.ToString(Read_Buf(18))
            Label_FRU_0013.Text = Convert.ToString(Read_Buf(19))
            Label_FRU_0014.Text = Convert.ToString(Read_Buf(20))
            Label_FRU_0015.Text = Convert.ToString(Read_Buf(21))
            Label_FRU_0016.Text = Convert.ToString(Read_Buf(22))
            Label_FRU_0017.Text = Convert.ToString(Read_Buf(23))
            Label_FRU_0018.Text = Convert.ToString(Read_Buf(24))
            Label_FRU_0019.Text = Convert.ToString(Read_Buf(25))
            Label_FRU_001A.Text = Convert.ToString(Read_Buf(26))
            Label_FRU_001B.Text = Convert.ToString(Read_Buf(27))
            Label_FRU_001C.Text = Convert.ToString(Read_Buf(28))
            Label_FRU_001D.Text = Convert.ToString(Read_Buf(29))
            Label_FRU_001E.Text = Convert.ToString(Read_Buf(30))
            Label_FRU_001F.Text = Convert.ToString(Read_Buf(31))

            Label_FRU_0020.Text = Convert.ToString(Read_Buf(32))
            Label_FRU_0021.Text = Convert.ToString(Read_Buf(33))
            Label_FRU_0022.Text = Convert.ToString(Read_Buf(34))
            Label_FRU_0023.Text = Convert.ToString(Read_Buf(35))
            Label_FRU_0024.Text = Convert.ToString(Read_Buf(36))
            Label_FRU_0025.Text = Convert.ToString(Read_Buf(37))
            Label_FRU_0026.Text = Convert.ToString(Read_Buf(38))
            Label_FRU_0027.Text = Convert.ToString(Read_Buf(39))
            Label_FRU_0028.Text = Convert.ToString(Read_Buf(40))
            Label_FRU_0029.Text = Convert.ToString(Read_Buf(41))
            Label_FRU_002A.Text = Convert.ToString(Read_Buf(42))
            Label_FRU_002B.Text = Convert.ToString(Read_Buf(43))
            Label_FRU_002C.Text = Convert.ToString(Read_Buf(44))
            Label_FRU_002D.Text = Convert.ToString(Read_Buf(45))
            Label_FRU_002E.Text = Convert.ToString(Read_Buf(46))
            Label_FRU_002F.Text = Convert.ToString(Read_Buf(47))

            Label_FRU_0030.Text = Convert.ToString(Read_Buf(48))
            Label_FRU_0031.Text = Convert.ToString(Read_Buf(49))
            Label_FRU_0032.Text = Convert.ToString(Read_Buf(50))
            Label_FRU_0033.Text = Convert.ToString(Read_Buf(51))
            Label_FRU_0034.Text = Convert.ToString(Read_Buf(52))
            Label_FRU_0035.Text = Convert.ToString(Read_Buf(53))
            Label_FRU_0036.Text = Convert.ToString(Read_Buf(54))
            Label_FRU_0037.Text = Convert.ToString(Read_Buf(55))
            Label_FRU_0038.Text = Convert.ToString(Read_Buf(56))
            Label_FRU_0039.Text = Convert.ToString(Read_Buf(57))
            Label_FRU_003A.Text = Convert.ToString(Read_Buf(58))
            Label_FRU_003B.Text = Convert.ToString(Read_Buf(59))
            Label_FRU_003C.Text = Convert.ToString(Read_Buf(60))
            Label_FRU_003D.Text = Convert.ToString(Read_Buf(61))
            Label_FRU_003E.Text = Convert.ToString(Read_Buf(62))
            Label_FRU_003F.Text = Convert.ToString(Read_Buf(63))

            Label_FRU_0040.Text = Convert.ToString(Read_Buf(64))
            Label_FRU_0041.Text = Convert.ToString(Read_Buf(65))
            Label_FRU_0042.Text = Convert.ToString(Read_Buf(66))
            Label_FRU_0043.Text = Convert.ToString(Read_Buf(67))
            Label_FRU_0044.Text = Convert.ToString(Read_Buf(68))
            Label_FRU_0045.Text = Convert.ToString(Read_Buf(69))
            Label_FRU_0046.Text = Convert.ToString(Read_Buf(70))
            Label_FRU_0047.Text = Convert.ToString(Read_Buf(71))
            Label_FRU_0048.Text = Convert.ToString(Read_Buf(72))
            Label_FRU_0049.Text = Convert.ToString(Read_Buf(73))
            Label_FRU_004A.Text = Convert.ToString(Read_Buf(74))
            Label_FRU_004B.Text = Convert.ToString(Read_Buf(75))
            Label_FRU_004C.Text = Convert.ToString(Read_Buf(76))
            Label_FRU_004D.Text = Convert.ToString(Read_Buf(77))
            Label_FRU_004E.Text = Convert.ToString(Read_Buf(78))
            Label_FRU_004F.Text = Convert.ToString(Read_Buf(79))

            Label_FRU_0050.Text = Convert.ToString(Read_Buf(80))
            Label_FRU_0051.Text = Convert.ToString(Read_Buf(81))
            Label_FRU_0052.Text = Convert.ToString(Read_Buf(82))
            Label_FRU_0053.Text = Convert.ToString(Read_Buf(83))
            Label_FRU_0054.Text = Convert.ToString(Read_Buf(84))
            Label_FRU_0055.Text = Convert.ToString(Read_Buf(85))
            Label_FRU_0056.Text = Convert.ToString(Read_Buf(86))
            Label_FRU_0057.Text = Convert.ToString(Read_Buf(87))
            Label_FRU_0058.Text = Convert.ToString(Read_Buf(88))
            Label_FRU_0059.Text = Convert.ToString(Read_Buf(89))
            Label_FRU_005A.Text = Convert.ToString(Read_Buf(90))
            Label_FRU_005B.Text = Convert.ToString(Read_Buf(91))
            Label_FRU_005C.Text = Convert.ToString(Read_Buf(92))
            Label_FRU_005D.Text = Convert.ToString(Read_Buf(93))
            Label_FRU_005E.Text = Convert.ToString(Read_Buf(94))
            Label_FRU_005F.Text = Convert.ToString(Read_Buf(95))

            Label_FRU_0060.Text = Convert.ToString(Read_Buf(96))
            Label_FRU_0061.Text = Convert.ToString(Read_Buf(97))
            Label_FRU_0062.Text = Convert.ToString(Read_Buf(98))
            Label_FRU_0063.Text = Convert.ToString(Read_Buf(99))
            Label_FRU_0064.Text = Convert.ToString(Read_Buf(100))
            Label_FRU_0065.Text = Convert.ToString(Read_Buf(101))
            Label_FRU_0066.Text = Convert.ToString(Read_Buf(102))
            Label_FRU_0067.Text = Convert.ToString(Read_Buf(103))
            Label_FRU_0068.Text = Convert.ToString(Read_Buf(104))
            Label_FRU_0069.Text = Convert.ToString(Read_Buf(105))
            Label_FRU_006A.Text = Convert.ToString(Read_Buf(106))
            Label_FRU_006B.Text = Convert.ToString(Read_Buf(107))
            Label_FRU_006C.Text = Convert.ToString(Read_Buf(108))
            Label_FRU_006D.Text = Convert.ToString(Read_Buf(109))
            Label_FRU_006E.Text = Convert.ToString(Read_Buf(110))
            Label_FRU_006F.Text = Convert.ToString(Read_Buf(111))

            Label_FRU_0070.Text = Convert.ToString(Read_Buf(112))
            Label_FRU_0071.Text = Convert.ToString(Read_Buf(113))
            Label_FRU_0072.Text = Convert.ToString(Read_Buf(114))
            Label_FRU_0073.Text = Convert.ToString(Read_Buf(115))
            Label_FRU_0074.Text = Convert.ToString(Read_Buf(116))
            Label_FRU_0075.Text = Convert.ToString(Read_Buf(117))
            Label_FRU_0076.Text = Convert.ToString(Read_Buf(118))
            Label_FRU_0077.Text = Convert.ToString(Read_Buf(119))
            Label_FRU_0078.Text = Convert.ToString(Read_Buf(120))
            Label_FRU_0079.Text = Convert.ToString(Read_Buf(121))
            Label_FRU_007A.Text = Convert.ToString(Read_Buf(122))
            Label_FRU_007B.Text = Convert.ToString(Read_Buf(123))
            Label_FRU_007C.Text = Convert.ToString(Read_Buf(124))
            Label_FRU_007D.Text = Convert.ToString(Read_Buf(125))
            Label_FRU_007E.Text = Convert.ToString(Read_Buf(126))
            Label_FRU_007F.Text = Convert.ToString(Read_Buf(127))

            Label_FRU_0080.Text = Convert.ToString(Read_Buf(128))
            Label_FRU_0081.Text = Convert.ToString(Read_Buf(129))
            Label_FRU_0082.Text = Convert.ToString(Read_Buf(130))
            Label_FRU_0083.Text = Convert.ToString(Read_Buf(131))
            Label_FRU_0084.Text = Convert.ToString(Read_Buf(132))
            Label_FRU_0085.Text = Convert.ToString(Read_Buf(133))
            Label_FRU_0086.Text = Convert.ToString(Read_Buf(134))
            Label_FRU_0087.Text = Convert.ToString(Read_Buf(135))
            Label_FRU_0088.Text = Convert.ToString(Read_Buf(136))
            Label_FRU_0089.Text = Convert.ToString(Read_Buf(137))
            Label_FRU_008A.Text = Convert.ToString(Read_Buf(138))
            Label_FRU_008B.Text = Convert.ToString(Read_Buf(139))
            Label_FRU_008C.Text = Convert.ToString(Read_Buf(140))
            Label_FRU_008D.Text = Convert.ToString(Read_Buf(141))
            Label_FRU_008E.Text = Convert.ToString(Read_Buf(142))
            Label_FRU_008F.Text = Convert.ToString(Read_Buf(143))

            Label_FRU_0090.Text = Convert.ToString(Read_Buf(144))
            Label_FRU_0091.Text = Convert.ToString(Read_Buf(145))
            Label_FRU_0092.Text = Convert.ToString(Read_Buf(146))
            Label_FRU_0093.Text = Convert.ToString(Read_Buf(147))
            Label_FRU_0094.Text = Convert.ToString(Read_Buf(148))
            Label_FRU_0095.Text = Convert.ToString(Read_Buf(149))
            Label_FRU_0096.Text = Convert.ToString(Read_Buf(150))
            Label_FRU_0097.Text = Convert.ToString(Read_Buf(151))
            Label_FRU_0098.Text = Convert.ToString(Read_Buf(152))
            Label_FRU_0099.Text = Convert.ToString(Read_Buf(153))
            Label_FRU_009A.Text = Convert.ToString(Read_Buf(154))
            Label_FRU_009B.Text = Convert.ToString(Read_Buf(155))
            Label_FRU_009C.Text = Convert.ToString(Read_Buf(156))
            Label_FRU_009D.Text = Convert.ToString(Read_Buf(157))
            Label_FRU_009E.Text = Convert.ToString(Read_Buf(158))
            Label_FRU_009F.Text = Convert.ToString(Read_Buf(159))

            Label_FRU_00A0.Text = Convert.ToString(Read_Buf(160))
            Label_FRU_00A1.Text = Convert.ToString(Read_Buf(161))
            Label_FRU_00A2.Text = Convert.ToString(Read_Buf(162))
            Label_FRU_00A3.Text = Convert.ToString(Read_Buf(163))
            Label_FRU_00A4.Text = Convert.ToString(Read_Buf(164))
            Label_FRU_00A5.Text = Convert.ToString(Read_Buf(165))
            Label_FRU_00A6.Text = Convert.ToString(Read_Buf(166))
            Label_FRU_00A7.Text = Convert.ToString(Read_Buf(167))
            Label_FRU_00A8.Text = Convert.ToString(Read_Buf(168))
            Label_FRU_00A9.Text = Convert.ToString(Read_Buf(169))
            Label_FRU_00AA.Text = Convert.ToString(Read_Buf(170))
            Label_FRU_00AB.Text = Convert.ToString(Read_Buf(171))
            Label_FRU_00AC.Text = Convert.ToString(Read_Buf(172))
            Label_FRU_00AD.Text = Convert.ToString(Read_Buf(173))
            Label_FRU_00AE.Text = Convert.ToString(Read_Buf(174))
            Label_FRU_00AF.Text = Convert.ToString(Read_Buf(175))

            Label_FRU_00B0.Text = Convert.ToString(Read_Buf(176))
            Label_FRU_00B1.Text = Convert.ToString(Read_Buf(177))
            Label_FRU_00B2.Text = Convert.ToString(Read_Buf(178))
            Label_FRU_00B3.Text = Convert.ToString(Read_Buf(179))
            Label_FRU_00B4.Text = Convert.ToString(Read_Buf(180))
            Label_FRU_00B5.Text = Convert.ToString(Read_Buf(181))
            Label_FRU_00B6.Text = Convert.ToString(Read_Buf(182))
            Label_FRU_00B7.Text = Convert.ToString(Read_Buf(183))
            Label_FRU_00B8.Text = Convert.ToString(Read_Buf(184))
            Label_FRU_00B9.Text = Convert.ToString(Read_Buf(185))
            Label_FRU_00BA.Text = Convert.ToString(Read_Buf(186))
            Label_FRU_00BB.Text = Convert.ToString(Read_Buf(187))
            Label_FRU_00BC.Text = Convert.ToString(Read_Buf(188))
            Label_FRU_00BD.Text = Convert.ToString(Read_Buf(189))
            Label_FRU_00BE.Text = Convert.ToString(Read_Buf(190))
            Label_FRU_00BF.Text = Convert.ToString(Read_Buf(191))


            Label_FRU_00C0.Text = Convert.ToString(Read_Buf(192))
            Label_FRU_00C1.Text = Convert.ToString(Read_Buf(193))
            Label_FRU_00C2.Text = Convert.ToString(Read_Buf(194))
            Label_FRU_00C3.Text = Convert.ToString(Read_Buf(195))
            Label_FRU_00C4.Text = Convert.ToString(Read_Buf(196))
            Label_FRU_00C5.Text = Convert.ToString(Read_Buf(197))
            Label_FRU_00C6.Text = Convert.ToString(Read_Buf(198))
            Label_FRU_00C7.Text = Convert.ToString(Read_Buf(199))
            Label_FRU_00C8.Text = Convert.ToString(Read_Buf(200))
            Label_FRU_00C9.Text = Convert.ToString(Read_Buf(201))
            Label_FRU_00CA.Text = Convert.ToString(Read_Buf(202))
            Label_FRU_00CB.Text = Convert.ToString(Read_Buf(203))
            Label_FRU_00CC.Text = Convert.ToString(Read_Buf(204))
            Label_FRU_00CD.Text = Convert.ToString(Read_Buf(205))
            Label_FRU_00CE.Text = Convert.ToString(Read_Buf(206))
            Label_FRU_00CF.Text = Convert.ToString(Read_Buf(207))


            Label_FRU_00D0.Text = Convert.ToString(Read_Buf(208))
            Label_FRU_00D1.Text = Convert.ToString(Read_Buf(209))
            Label_FRU_00D2.Text = Convert.ToString(Read_Buf(210))
            Label_FRU_00D3.Text = Convert.ToString(Read_Buf(211))
            Label_FRU_00D4.Text = Convert.ToString(Read_Buf(212))
            Label_FRU_00D5.Text = Convert.ToString(Read_Buf(213))
            Label_FRU_00D6.Text = Convert.ToString(Read_Buf(214))
            Label_FRU_00D7.Text = Convert.ToString(Read_Buf(215))
            Label_FRU_00D8.Text = Convert.ToString(Read_Buf(216))
            Label_FRU_00D9.Text = Convert.ToString(Read_Buf(217))
            Label_FRU_00DA.Text = Convert.ToString(Read_Buf(218))
            Label_FRU_00DB.Text = Convert.ToString(Read_Buf(219))
            Label_FRU_00DC.Text = Convert.ToString(Read_Buf(220))
            Label_FRU_00DD.Text = Convert.ToString(Read_Buf(221))
            Label_FRU_00DE.Text = Convert.ToString(Read_Buf(222))
            Label_FRU_00DF.Text = Convert.ToString(Read_Buf(223))

            Label_FRU_00E0.Text = Convert.ToString(Read_Buf(224))
            Label_FRU_00E1.Text = Convert.ToString(Read_Buf(225))
            Label_FRU_00E2.Text = Convert.ToString(Read_Buf(226))
            Label_FRU_00E3.Text = Convert.ToString(Read_Buf(227))
            Label_FRU_00E4.Text = Convert.ToString(Read_Buf(228))
            Label_FRU_00E5.Text = Convert.ToString(Read_Buf(229))
            Label_FRU_00E6.Text = Convert.ToString(Read_Buf(230))
            Label_FRU_00E7.Text = Convert.ToString(Read_Buf(231))
            Label_FRU_00E8.Text = Convert.ToString(Read_Buf(232))
            Label_FRU_00E9.Text = Convert.ToString(Read_Buf(233))
            Label_FRU_00EA.Text = Convert.ToString(Read_Buf(234))
            Label_FRU_00EB.Text = Convert.ToString(Read_Buf(235))
            Label_FRU_00EC.Text = Convert.ToString(Read_Buf(236))
            Label_FRU_00ED.Text = Convert.ToString(Read_Buf(237))
            Label_FRU_00EE.Text = Convert.ToString(Read_Buf(238))
            Label_FRU_00EF.Text = Convert.ToString(Read_Buf(239))

            Label_FRU_00F0.Text = Convert.ToString(Read_Buf(240))
            Label_FRU_00F1.Text = Convert.ToString(Read_Buf(241))
            Label_FRU_00F2.Text = Convert.ToString(Read_Buf(242))
            Label_FRU_00F3.Text = Convert.ToString(Read_Buf(243))
            Label_FRU_00F4.Text = Convert.ToString(Read_Buf(244))
            Label_FRU_00F5.Text = Convert.ToString(Read_Buf(245))
            Label_FRU_00F6.Text = Convert.ToString(Read_Buf(246))
            Label_FRU_00F7.Text = Convert.ToString(Read_Buf(247))
            Label_FRU_00F8.Text = Convert.ToString(Read_Buf(248))
            Label_FRU_00F9.Text = Convert.ToString(Read_Buf(249))
            Label_FRU_00FA.Text = Convert.ToString(Read_Buf(250))
            Label_FRU_00FB.Text = Convert.ToString(Read_Buf(251))
            Label_FRU_00FC.Text = Convert.ToString(Read_Buf(252))
            Label_FRU_00FD.Text = Convert.ToString(Read_Buf(253))
            Label_FRU_00FE.Text = Convert.ToString(Read_Buf(254))
            Label_FRU_00FF.Text = Convert.ToString(Read_Buf(255))

        End If
#End If



#End If
    End Sub

    Private Sub Button_FRU_File_Click_1(sender As Object, e As EventArgs) Handles Button_FRU_File.Click

    End Sub

    Private Sub Button_Read_All_Click(sender As Object, e As EventArgs) Handles Button_Read_All.Click

        Dim strFile As String = "C:\wkp\ErrorLog_" & DateTime.Today.ToString("dd-MMM-yyyy") & ".txt"
        ' Dim sw As StreamWriter
        Dim fs As FileStream = Nothing


        ' Dim EEPROM_Addr As Byte = Demo_Slave_ee_Addr '&HA1 '&HA5 ' A0 A2 A4 ' AddrW+1
        Dim Return_Str As String = ""
        Dim ByteNumber As Integer = &HFF
        Dim FILE_NAME As String = "C:\wkp\7 600W\1 pmbus\pmbus vb code\test1.txt"


#If 1 Then' to file
        If System.IO.File.Exists(FILE_NAME) = True Then

            Dim objWriter As New System.IO.StreamWriter(FILE_NAME)
            ' objWriter.Write(TextBox1.Text)
            If (PICkitS.I2CM.Receive(Demo_Slave_ee_Addr + 1, ByteNumber, Read_Buf, Return_Str)) Then
                For index As Integer = 0 To ByteNumber
                    objWriter.Write(Convert.ToString(Read_Buf(index)))
                Next
                objWriter.Close()
                MessageBox.Show("Text written to file")
            Else
#If 1 Then
                For index As Integer = 0 To 10 'ByteNumber
                    objWriter.Write(Convert.ToString(Read_Buf(index)))
                    objWriter.Write(" ")
                    objWriter.Write(Convert.ToString("17"))
                    objWriter.Write(" ")
                Next
                objWriter.Close()

#End If
                MessageBox.Show("File Does Not Exist!", "LiteOn CIPS PMBUS")
            End If
        End If
#End If

#If 0 Then
        Using sw As New StreamWriter(File.Open(strFile, FileMode.OpenOrCreate))
            sw.WriteLine(
        IIf(fileExists,
            "Error Message in  Occured at-- " & DateTime.Now,
            "Start Error Log for today"))
        End Using
#End If

    End Sub


    Private Sub BTN_FRU_Read_Click(sender As Object, e As EventArgs) Handles BTN_FRU_Read.Click
        ' Dim Demo_Slave_ee_Addr As Byte = eeprom_Addr_w
        ' determined by Demo board firmware, &H says this is a hex number
        Dim Word_Addr As Byte = Convert.ToByte(TextBox_Write_EE_Word_Addr.Text, 16)
        ' assume value in textbox is hex
        Dim Byte_Count As Byte = Convert.ToByte(TextBox_Read_EE_Byte_Count.Text, 16)
        ' assume value in textbox is hex
        Dim DataArray(256) As Byte
        ' will store returned data here
        Dim Return_Str As String = ""
        ' returns string representation of command - we won't use this
        Dim Display_Str As String = ""
        ' will display results with this
        Dim index As Integer
        ' used as counter
        '
        ' clear array
        For index = 0 To Byte_Count - 1
            DataArray(index) = 0
        Next
        '
        If (PICkitS.I2CM.Read(Demo_Slave_ee_Addr, Word_Addr, Byte_Count, DataArray, Return_Str)) Then
            ' successful, display results
            For index = 0 To Byte_Count - 1
                Display_Str += Convert.ToString(DataArray(index), 16) & " "
            Next
            RichTextBox1.Text += Display_Str & vbCrLf
        Else
            RichTextBox1.Text += "Error reading Demo Board EE" & vbCrLf
            PICkitS.Device.Reset_Control_Block()  ' clear any errors in PKSA
        End If
    End Sub


    Private Sub Button_Write_Page_Click(sender As Object, e As EventArgs) Handles Button_Write_Page.Click
        RichTextBox1.Text = TextBox_Write_EE_Byte2.Text
    End Sub


    Private Sub Button_Clear_FRU_Click_2(sender As Object, e As EventArgs) Handles Button_Clear_FRU.Click
        Dim ClearData_d As String
        ClearData_d = "0"
#If 0 Then
        Label_FRU_0000.Text = ClearData_d
        Label_FRU_0001.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0002.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0003.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0004.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0005.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0006.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0007.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0008.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0009.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_000A.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_000B.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_000C.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_000D.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_000E.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_000F.Text = ClearData_d 'Convert.ToByte(ClearData_d)

        Label_FRU_0010.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0011.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0012.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0013.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0014.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0015.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0016.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0017.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0018.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0019.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_001A.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_001B.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_001C.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_001D.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_001E.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_001F.Text = ClearData_d 'Convert.ToByte(ClearData_d)

        Label_FRU_0020.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0021.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0022.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0023.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0024.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0025.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0026.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0027.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0028.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0029.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_002A.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_002B.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_002C.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_002D.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_002E.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_002F.Text = ClearData_d 'Convert.ToByte(ClearData_d)

        Label_FRU_0030.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0031.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0032.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0033.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0034.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0035.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0036.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0037.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0038.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0039.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_003A.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_003B.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_003C.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_003D.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_003E.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_003F.Text = ClearData_d 'Convert.ToByte(ClearData_d)

        Label_FRU_0040.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0041.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0042.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0043.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0044.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0045.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0046.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0047.Text = ClearData_d 'Convert.ToByte(ClearData_d)
        Label_FRU_0048.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0049.Text = Convert.ToByte(ClearData_d)
        Label_FRU_004A.Text = Convert.ToByte(ClearData_d)
        Label_FRU_004B.Text = Convert.ToByte(ClearData_d)
        Label_FRU_004C.Text = Convert.ToByte(ClearData_d)
        Label_FRU_004D.Text = Convert.ToByte(ClearData_d)
        Label_FRU_004E.Text = Convert.ToByte(ClearData_d)
        Label_FRU_004F.Text = Convert.ToByte(ClearData_d)

        Label_FRU_0050.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0051.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0052.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0053.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0054.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0055.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0056.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0057.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0058.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0059.Text = Convert.ToByte(ClearData_d)
        Label_FRU_005A.Text = Convert.ToByte(ClearData_d)
        Label_FRU_005B.Text = Convert.ToByte(ClearData_d)
        Label_FRU_005C.Text = Convert.ToByte(ClearData_d)
        Label_FRU_005D.Text = Convert.ToByte(ClearData_d)
        Label_FRU_005E.Text = Convert.ToByte(ClearData_d)
        Label_FRU_005F.Text = Convert.ToByte(ClearData_d)


        Label_FRU_0060.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0061.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0062.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0063.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0064.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0065.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0066.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0067.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0068.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0069.Text = Convert.ToByte(ClearData_d)
        Label_FRU_006A.Text = Convert.ToByte(ClearData_d)
        Label_FRU_006B.Text = Convert.ToByte(ClearData_d)
        Label_FRU_006C.Text = Convert.ToByte(ClearData_d)
        Label_FRU_006D.Text = Convert.ToByte(ClearData_d)
        Label_FRU_006E.Text = Convert.ToByte(ClearData_d)
        Label_FRU_006F.Text = Convert.ToByte(ClearData_d)

        Label_FRU_0070.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0071.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0072.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0073.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0074.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0075.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0076.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0077.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0078.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0079.Text = Convert.ToByte(ClearData_d)
        Label_FRU_007A.Text = Convert.ToByte(ClearData_d)
        Label_FRU_007B.Text = Convert.ToByte(ClearData_d)
        Label_FRU_007C.Text = Convert.ToByte(ClearData_d)
        Label_FRU_007D.Text = Convert.ToByte(ClearData_d)
        Label_FRU_007E.Text = Convert.ToByte(ClearData_d)
        Label_FRU_007F.Text = Convert.ToByte(ClearData_d)

        Label_FRU_0080.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0081.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0082.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0083.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0084.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0085.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0086.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0087.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0088.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0089.Text = Convert.ToByte(ClearData_d)
        Label_FRU_008A.Text = Convert.ToByte(ClearData_d)
        Label_FRU_008B.Text = Convert.ToByte(ClearData_d)
        Label_FRU_008C.Text = Convert.ToByte(ClearData_d)
        Label_FRU_008D.Text = Convert.ToByte(ClearData_d)
        Label_FRU_008E.Text = Convert.ToByte(ClearData_d)
        Label_FRU_008F.Text = Convert.ToByte(ClearData_d)


        Label_FRU_0090.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0091.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0092.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0093.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0094.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0095.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0096.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0097.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0098.Text = Convert.ToByte(ClearData_d)
        Label_FRU_0099.Text = Convert.ToByte(ClearData_d)
        Label_FRU_009A.Text = Convert.ToByte(ClearData_d)
        Label_FRU_009B.Text = Convert.ToByte(ClearData_d)
        Label_FRU_009C.Text = Convert.ToByte(ClearData_d)
        Label_FRU_009D.Text = Convert.ToByte(ClearData_d)
        Label_FRU_009E.Text = Convert.ToByte(ClearData_d)
        Label_FRU_009F.Text = Convert.ToByte(ClearData_d)

        Label_FRU_00A0.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A1.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A2.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A3.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A4.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A5.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A6.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A7.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A8.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00A9.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00AA.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00AB.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00AC.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00AD.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00AE.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00AF.Text = Convert.ToByte(ClearData_d)

        Label_FRU_00B0.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B1.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B2.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B3.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B4.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B5.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B6.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B7.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B8.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00B9.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00BA.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00BB.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00BC.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00BD.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00BE.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00BF.Text = Convert.ToByte(ClearData_d)



        Label_FRU_00C0.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C1.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C2.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C3.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C4.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C5.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C6.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C7.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C8.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00C9.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00CA.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00CB.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00CC.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00CD.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00CE.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00CF.Text = Convert.ToByte(ClearData_d)



        Label_FRU_00D0.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D1.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D2.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D3.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D4.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D5.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D6.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D7.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D8.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00D9.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00DA.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00DB.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00DC.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00DD.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00DE.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00DF.Text = Convert.ToByte(ClearData_d)

        Label_FRU_00E0.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E1.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E2.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E3.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E4.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E5.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E6.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E7.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E8.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00E9.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00EA.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00EB.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00EC.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00ED.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00EE.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00EF.Text = Convert.ToByte(ClearData_d)


        Label_FRU_00F0.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F1.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F2.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F3.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F4.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F5.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F6.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F7.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F8.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00F9.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00FA.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00FB.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00FC.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00FD.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00FE.Text = Convert.ToByte(ClearData_d)
        Label_FRU_00FF.Text = Convert.ToByte(ClearData_d)
#End If
    End Sub
#End Region

#Region "Menu"



    Private Sub HelpToolStripMenuItem1_Click(sender As Object, e As EventArgs)
#If 0 Then
        'Option Explicit On
        Dim oWord As pdf.Application
        Dim oDoc As PDF.Document
        oWord = CreateObject("Word.Application")
        oWord.Visible = True
        oDoc = oWord.Documents.Add("C:\wordfile.docx")
#End If
        System.Diagnostics.Process.Start("C:\wkp\7 600W\0 Liteon\Can API.pdf")
    End Sub



    Private Sub HelpToolStripMenuItem_Click(sender As Object, e As EventArgs)

    End Sub



    Private Sub InstructionToolStripMenuItem_Click(sender As Object, e As EventArgs)
        System.Diagnostics.Process.Start("C:\wkp\7 600W\0 Liteon\Can API.pdf") ' help
    End Sub


    Private Sub ConstatDataToolStripMenuItem_DragEnter(sender As Object, e As DragEventArgs)

    End Sub



    Private Sub Tag_Help_Click(sender As Object, e As EventArgs) Handles Tag_Help.Click
        System.Diagnostics.Process.Start("C:\wkp\7 600W\0 Liteon\Can API.pdf") ' help
    End Sub













#If 1 Then

    Private Sub TabControl1_SelectedIndexChanged(ByVal sender As Object,
                                              ByVal e As System.EventArgs) Handles TabControl1.SelectedIndexChanged

        ListBox_adc.SelectedIndex = 0
        ListBox_index.SelectedIndex = 0
        Select Case TabControl1.SelectedIndex
            Case 1
                RichTextBox1.Text = ""
                Append_Text1("Read status" & vbCrLf)
            Case 4
                Append_Text1("Constant status" & vbCrLf)
            Case 5
                RichTextBox1.Text = ""
                RichTextBox1.BackColor = Color.White
                RichTextBox1.ForeColor = Color.DarkBlue

                Append_Text1("Welcome to Calibration interface ! Please to follow the instruction as below" & vbCrLf)
                'Append_Text1("Please to follow the instruction as below" & vbCrLf)
        End Select

#If 0 Then

            Dim indexOfSelectedTab As Integer = MyTabControl.SelectedIndex
        Dim selectedTab As System.Windows.Forms.TabPage = MyTabControl.SelectedTab

#End If

    End Sub

    Private Sub Button27_Click(sender As Object, e As EventArgs) Handles Button27.Click

    End Sub

    Private Sub ListBox_index_SelectedIndexChanged(sender As Object, e As EventArgs) Handles ListBox_index.SelectedIndexChanged

        Get_Index()
    End Sub


#If 0 Then

    Private Sub TabControl1_MouseClick(sender As Object,
                                    e As System.Windows.Forms.MouseEventArgs) _
            Handles TabControl1.MouseClick
        MsgBox("here tab m")

    End Sub
#End If
#End If












#End Region


End Class





