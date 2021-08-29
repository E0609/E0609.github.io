Imports System.IO
Imports System.Text
Imports System.BitConverter
Imports System.Threading
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
    Dim Pic_Kit_Error As Boolean = False

    Dim RTB1_Line_Num As UInteger = 0
    Dim Read_Buf(64) As Byte
    Dim Write_Buf(72) As Byte
    Dim Read_Buf_Str As String = ""
    Dim Write_Buf_Str As String = ""
    Dim I2C_Err_Flag As Boolean = False
    Dim Slave_Addr As Byte
    Dim Slave_Addr_Rd As Byte
    Dim Poll_Button_Latched As Boolean
    Dim Test_Button_Latched As Boolean
    Dim CRC8_Byte As Byte = 0
    Dim PEC_Err_Flag As Boolean = False
    Dim Query As Boolean = False
    Dim Query_Delay As Byte

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
#Region "Controls"
    Private Sub Form1_Load(sender As System.Object, e As System.EventArgs) Handles MyBase.Load

        Init_Pmbus_DGV(0)
        Init_Pmbus_Cnst_DGV(0)
        Init_Pmbus_Cnst_1_DGV(0)
        Init_Pmbus_MFR_DGV(0)
        Init_Pmbus_Log_DGV(0)

        Me.WindowState = FormWindowState.Maximized

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
#If 0 Then
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
    Private Sub Append_Text1(ByVal Str_Data As String)
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
    Private Sub Read_All(ByVal Null_Data As Byte)
        Dim TabName As String = TabControl1.SelectedTab.Name
        If TabName = "TabPage1" Then
            Append_Text1("Started to Read PMBus Status Data........" & vbCrLf)
            Update_Pmbus_Data(0)
            'Send_Byte(&H3)
        End If
    End Sub
    Public Function Read_Byte(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = "00 00"
        Slave_Addr = NumericUpDown1.Value

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
        Slave_Addr = NumericUpDown1.Value

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

        Slave_Addr = NumericUpDown1.Value

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

        Slave_Addr = NumericUpDown1.Value

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
        Slave_Addr = NumericUpDown1.Value

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
                End If
            End If
        End If
        Return Return_Str
    End Function

    Public Function Read_Block1(ByVal Cmd_Addr As Byte, ByVal Byte_Count As Byte) ' Read With Length Byte - Count Including PEC 
        Dim Return_Str As String = "00"
        Slave_Addr = NumericUpDown1.Value

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
        Slave_Addr = NumericUpDown1.Value

        Pmb_Hex_Data = "-"
        Pmb_Act_Data = "-"

        Byte_Count = Byte_Count + 1 ' without length byte

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
                End If
            End If
        End If
        Return Return_Str
    End Function

    Public Function Process_Block(ByVal Cmd_Addr As Byte, ByVal Byte_Count As Byte)
        Dim Return_Str As String = "00"
        ' successful, display results
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
        Slave_Addr = NumericUpDown1.Value
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
            If Win_I2C_Error = False Then
                Dim Write_Sta As Byte = I2CWriteArray(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf(0))
                If Write_Sta = 0 Then
                    Append_Text1("Send Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Win_I2C_Error = True
                    Write_Buf(0) = 0
                    Write_Buf(1) = 0
                    Write_Buf(2) = 0
                End If
            End If
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
        Slave_Addr = NumericUpDown1.Value
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
            If Win_I2C_Error = False Then
                Dim Write_Sta As Byte = I2CWriteArray(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf(0))
                If Write_Sta = 0 Then
                    ' successful, display results     
                    Append_Text1("Write Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Win_I2C_Error = True
                    Write_Buf(0) = 0
                    Write_Buf(1) = 0
                    Write_Buf(2) = 0
                End If
            End If
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
    Private Sub Write_Word(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = ""
        Slave_Addr = NumericUpDown1.Value
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
            If Win_I2C_Error = False Then
                Dim Write_Sta As Byte = I2CWriteArray(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf(0))
                If Write_Sta = 0 Then
                    ' successful, display results     
                    Append_Text1("Write Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Win_I2C_Error = True
                    Write_Buf(0) = 0
                    Write_Buf(1) = 0
                    Write_Buf(2) = 0
                End If
            End If
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
    Private Sub Write_Block(ByVal Cmd_Addr As Byte)
        Dim Return_Str As String = ""
        Dim i As Integer = 0
        Slave_Addr = NumericUpDown1.Value '&H64
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
            If Win_I2C_Error = False Then
                Dim Write_Sta As Byte = I2CWriteArray(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf(0))
                If Write_Sta = 0 Then
                    Append_Text1("Write Block Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Win_I2C_Error = True
                    For i = 0 To Write_Buf(0)
                        Write_Buf(i) = 0
                    Next
                End If
            End If
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
        Slave_Addr = NumericUpDown1.Value
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
            If Win_I2C_Error = False Then
                Dim Write_Sta As Byte = I2CWriteArray(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf(0))
                If Write_Sta = 0 Then
                    Append_Text1("Write Block Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Win_I2C_Error = True
                    For i = 0 To Write_Buf(0)
                        Write_Buf(i) = 0
                    Next
                End If
            End If
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


    Private Sub Write_Block_NL(ByVal Cmd_Addr As Byte, ByVal len As Byte)
        Dim Return_Str As String = ""
        Dim i As Integer = 0
        Slave_Addr = NumericUpDown1.Value
        'Compute CRC 8 
        CRC8_Byte = 0
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Slave_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Cmd_Addr, CRC8_Byte)
        CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(0), CRC8_Byte)
        For i = 1 To len - 1
            CRC8_Byte = PICkitS.Utilities.calculate_crc8(Write_Buf(i), CRC8_Byte)
        Next
        Write_Buf(i) = CRC8_Byte

        ' successful, display results      
        Return_Str = Convert.ToString(Write_Buf(0), 16).ToUpper
        If Not Return_Str.Length = 2 Then
            Return_Str = "0" & Return_Str
        End If
        Write_Buf_Str = Return_Str & " "

        For i = 1 To len - 1
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
            Data_Len = len + 1
        Else
            Data_Len = len + 0
        End If

        If Hardware_Selection = 1 Then
            If Win_I2C_Error = False Then
                Dim Write_Sta As Byte = I2CWriteArray(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf(0))
                If Write_Sta = 0 Then
                    Append_Text1("Write Block WO Length Byte Sucessful - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
                Else
                    Append_Text1("Error Writing Data to the Device" & vbCrLf)
                    Win_I2C_Error = True
                    For i = 0 To Write_Buf(0)
                        Write_Buf(i) = 0
                    Next
                End If
            End If
        ElseIf Hardware_Selection = 2 Then
            If Pic_Kit_Error = False Then
                If (PICkitS.I2CM.Write(Slave_Addr, Cmd_Addr, Data_Len, Write_Buf, Return_Str)) Then
                    Append_Text1("Write Block Sucessful WO Length Byte - " & Convert.ToString(Cmd_Addr, 16).ToUpper & " - " & Write_Buf_Str & vbCrLf)
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
#End Region
#End Region

#Region "Pmbus Controls & Functions"
#Region "Pmbus Controls"


    Private Sub CheckBox2_CheckedChanged(sender As System.Object, e As System.EventArgs) Handles CheckBox2.CheckedChanged
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

    Private Sub Button5_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button5.Click
        Send_Byte(&H3)
    End Sub


    Private Sub Button56_Click(sender As System.Object, e As System.EventArgs) Handles Button56.Click
        ' Page Command
        Write_Buf(0) = Convert.ToByte(NumericUpDown16.Value, 10)
        Page_sel = Convert.ToByte(NumericUpDown16.Value, 10)
        Write_Byte(&H0)
    End Sub


    Private Sub Button50_Click(sender As System.Object, e As System.EventArgs) Handles Button50.Click
        Write_Buf(0) = Convert.ToByte(NumericUpDown12.Value, 10)
        Write_Buf(1) = 0
        Write_Word(&H3B)
    End Sub
    Private Sub Button49_Click(sender As System.Object, e As System.EventArgs) Handles Button49.Click
        'Read Fan Duty
        Dim str As String = Read_Word(&H3B)
        TextBox51.Text = Pmb_Hex_Data
    End Sub



    Private Sub Button26_Click(sender As System.Object, e As System.EventArgs) Handles Button26.Click
        Write_Buf(0) = Convert.ToByte(NumericUpDown2.Value, 10)
        Write_Buf(1) = 0
        Write_Word(&HD0)
    End Sub


    Private Sub Button29_Click(sender As Object, e As EventArgs) Handles Button29.Click
        'Read Vbulk
        Dim str As String = Read_Word(&HD0)
        TextBox28.Text = Pmb_Hex_Data
    End Sub
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

    Private Sub Button23_Click(sender As System.Object, e As System.EventArgs) Handles Button23.Click
        'Dim Val As UInteger = 0
        'Val = Convert.ToUInt16(TextBox16.Text, 10)

        Write_Buf(0) = ON_OFF_Config
        Write_Byte(&H2)
    End Sub
    Private Sub Button24_Click(sender As System.Object, e As System.EventArgs) Handles Button24.Click
        'Read Fan Duty
        Dim str As String = Read_Byte(&H2)
        TextBox17.Text = Pmb_Hex_Data
    End Sub

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

    Private Sub Button17_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button17.Click
        'PSU ON
        Write_Buf(0) = &H80
        Write_Byte(&H1)
    End Sub
    Private Sub Button16_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button16.Click
        'PSU OFF
        Write_Buf(0) = &H0
        Write_Byte(&H1)
    End Sub
    Private Sub Button48_Click(sender As System.Object, e As System.EventArgs) Handles Button48.Click
        Append_Text1("Started to Read PMBus Constant Data........" & vbCrLf)
        Update_Pmbus_Constant(0)
    End Sub

    Private Sub Button19_Click(sender As System.Object, e As System.EventArgs) Handles Button19.Click
        Read_Block(&H86, 6)
        TextBox8.Text = (Read_Buf(1)) + (Read_Buf(2) * 256)
        TextBox11.Text = Read_Buf(3)
        TextBox12.Text = Read_Buf(4) + (Read_Buf(5) * 256) + (Read_Buf(6) * 65536)
    End Sub
    Private Sub Button20_Click(sender As System.Object, e As System.EventArgs) Handles Button20.Click
        Read_Block(&H87, 6)
        TextBox9.Text = (Read_Buf(1)) + (Read_Buf(2) * 256)
        TextBox13.Text = Read_Buf(3)
        TextBox14.Text = Read_Buf(4) + (Read_Buf(5) * 256) + (Read_Buf(6) * 65536)
    End Sub
    Private Sub Button22_Click(sender As System.Object, e As System.EventArgs) Handles Button22.Click
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
                    TextBox15.Text = str2 & Convert.ToString(Read_Buf(5), 16)

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
    Private Sub Button25_Click(sender As System.Object, e As System.EventArgs) Handles Button25.Click
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
    Private Sub CheckBox4_CheckedChanged(sender As System.Object, e As System.EventArgs) Handles CheckBox4.CheckedChanged
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub CheckBox72_CheckedChanged(sender As System.Object, e As System.EventArgs) Handles CheckBox72.CheckedChanged
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub CheckBox73_CheckedChanged(sender As System.Object, e As System.EventArgs) Handles CheckBox73.CheckedChanged
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub CheckBox74_CheckedChanged(sender As System.Object, e As System.EventArgs) Handles CheckBox74.CheckedChanged
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub CheckBox3_CheckedChanged(sender As System.Object, e As System.EventArgs) Handles CheckBox3.CheckedChanged
        Update_ON_OFF_Config(0)
    End Sub
    Private Sub Update_ON_OFF_Config(ByVal Word_Addr As Byte)
        'Dim ON_OFF_Config As Byte = 0

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

        Slave_Addr = NumericUpDown1.Value

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

        Slave_Addr = NumericUpDown1.Value

        Write_Buf(0) = 4 ' Count
        Write_Buf(1) = NumericUpDown1.Value ' Status Vout Command
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

        Slave_Addr = NumericUpDown1.Value

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

        Slave_Addr = NumericUpDown1.Value

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

        Slave_Addr = NumericUpDown1.Value

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

        Slave_Addr = NumericUpDown1.Value

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

        Slave_Addr = NumericUpDown1.Value

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

        Slave_Addr = NumericUpDown1.Value

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

    Private Sub Button105_Click(sender As System.Object, e As System.EventArgs) Handles Button105.Click

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

    Private Sub Button106_Click(sender As System.Object, e As System.EventArgs) Handles Button106.Click
        Dim Return_Str As String = ""
        Dim SMB_Mask As Integer = 0

        Slave_Addr = NumericUpDown1.Value

        Write_Buf(0) = Convert.ToByte(TextBox69.Text, 10) ' Count
        Write_Buf(1) = Convert.ToByte(NumericUpDown17.Value, 10) 'Page
        Write_Buf(2) = Convert.ToByte(TextBox68.Text, 10)  'Command Code

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

                    TextBox75.Text = Convert.ToString(Read_Buf(0), 10).ToUpper

                    If Read_Buf(0) > 1 Then
                        SMB_Mask = Read_Buf(1) + (Read_Buf(2) * 256)
                    Else
                        SMB_Mask = Read_Buf(1)
                    End If

                    TextBox71.Text = Convert.ToString(SMB_Mask, 10).ToUpper
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

    Private Sub Button15_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button15.Click
        If Capture_Data = False Then
            SaveFileDialog1.FileName = "Fan_Profile_xx'C_xL_xxx.csv"
            SaveFileDialog1.InitialDirectory = "C:\Users\Meravanagikiran\Documents"

            If SaveFileDialog1.ShowDialog() = System.Windows.Forms.DialogResult.OK Then
                Dim bytes() As Byte
                bytes = Encoding.ASCII.GetBytes("Date & Time,T Ambient,T Hotspot,T Outlet,Fan Speed" & vbCrLf)

                Log_File_Name = SaveFileDialog1.FileName
                My.Computer.FileSystem.WriteAllBytes(Log_File_Name, bytes, False)
                Button15.BackColor = Color.Green
                Data_Arr_Pntr = 0
                Capture_Data = True
                Capture_Data_Pntr = 0
            End If
        End If
    End Sub
    Private Sub Button6_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button6.Click
        If Capture_Data = True Then
            Capture_Data = False
            Button6.BackColor = Color.Yellow
        ElseIf Button15.BackColor = Color.Green Then
            Capture_Data = True
            Button6.BackColor = Color.Transparent
        End If
    End Sub
    Private Sub Button11_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button11.Click
        If Capture_Data = True Then
            Capture_Data = False
            Update_Log_to_file(0)
            Button15.BackColor = Color.Transparent
        End If
    End Sub
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


    Private Sub Init_Pmbus_Cnst_DGV(ByVal Null_Data As Byte)
        Dim Arr_Len As Byte = PMBus_Cnst_Struct.Length
        For Arr_Loc As Byte = 0 To Arr_Len - 1
            DataGridView4.Rows.Add(1)
            DataGridView4.Rows(Arr_Loc).Cells(0).Value() = Convert.ToString(PMBus_Cnst_Struct(Arr_Loc).Command, 16).ToUpper
            DataGridView4.Rows(Arr_Loc).Cells(1).Value() = PMBus_Cnst_Struct(Arr_Loc).Cmd_Name
        Next
    End Sub


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



    Private Sub Init_Pmbus_Log_DGV(ByVal Null_Data As Byte)
        Dim Arr_Len As Byte = PMBus_LOG_Struct.Length
        For Arr_Loc As Byte = 0 To Arr_Len - 1
            DataGridView_LOG.Rows.Add(1)
            DataGridView_LOG.Rows(Arr_Loc).Cells(0).Value() = Convert.ToString(PMBus_LOG_Struct(Arr_Loc).Command, 16).ToUpper
            DataGridView_LOG.Rows(Arr_Loc).Cells(1).Value() = PMBus_LOG_Struct(Arr_Loc).Cmd_Name
        Next
    End Sub


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
                        DataGridView4.Rows(Arr_Loc).Cells(4).Value() = Pmb_Act_Data
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

                End If

            End If
        Next
    End Sub


    Private Sub Update_Pmbus_Log(ByVal Null_Data As Byte)
        Dim str As String
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
            If Win_I2C_Error = True And Hardware_Selection = 1 Then
                Err_Rec_Count += 1
                If Err_Rec_Count > 200 Then ' Delay .5 Sec
                    Win_I2C_Error = False
                    Append_Text1("WIN I2C Hardware Re-Configured for I2C" & vbCrLf)
                    Enable_I2C()
                    Append_Text1("I2C Module Enabled as Master" & vbCrLf)
                    Dim Freq As Integer = GetI2CFrequency()
                    Append_Text1("I2C Frequency Set as " & Convert.ToString(Freq, 10) & " Khz" & vbCrLf)
                    Dim FW_Rev As Byte = GetFirmwareRevision()
                    Append_Text1("Firmware Revision: " & Convert.ToString(FW_Rev, 10) & vbCrLf)
                    Dim Dll_Ver As Long = Get_DLL_Version()
                    Append_Text1("DLL Version: " & Convert.ToString(Dll_Ver, 10) & vbCrLf)
                    Err_Rec_Count = 0
                    RichTextBox1.BackColor = Color.White

                    'Read_Pri_Flag = False
                ElseIf Err_Rec_Count = 1 Then
                    RichTextBox1.BackColor = Color.Khaki
                    Disable_I2C()
                End If
            ElseIf Pic_Kit_Error = True And Hardware_Selection = 2 Then
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
                    Button15.BackColor = Color.Transparent
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
        '  For Hardware detection 
        If Hardware_Detected = 0 Then ' Hardware not detcted yet
            Win_I2C_Count = 0          'GetNumberOfDevices()
            PKSA_Count = PICkitS.Device.How_Many_PICkitSerials_Are_Attached()
            If Win_I2C_Count >= 1 And (Hardware_Selection = 0 Or Hardware_Selection = 1) Then
                If Win_I2C_Count = 1 Then
                    Hardware_Detected = 1
                    Hardware_Selection = 1
                    ToolStripStatusLabel1.Text = "WIN-I2C Hardware Detected"
                    ToolStripStatusLabel1.BackColor = Color.YellowGreen
                    TabControl1.Enabled = True
                    Poll.Enabled = True
                    ReadOnce.Enabled = True
                    PICkitConnect.Enabled = False
                    Append_Text1("WIN I2C Hardware Connected & Configured for I2C" & vbCrLf)
                    Win_I2C_Count = Enable_I2C()
                    Append_Text1("I2C Module Enabled as Master" & vbCrLf)
                    Dim Freq As Integer = GetI2CFrequency()
                    Append_Text1("I2C Frequency Set as " & Convert.ToString(Freq, 10) & " Khz" & vbCrLf)
                    Dim FW_Rev As Byte = GetFirmwareRevision()
                    Append_Text1("Firmware Revision: " & Convert.ToString(FW_Rev, 10) & vbCrLf)
                    Dim Dll_Ver As Long = Get_DLL_Version()
                    Append_Text1("DLL Version: " & Convert.ToString(Dll_Ver, 10) & vbCrLf)
                Else
                    ToolStripStatusLabel1.Text = " Invalid Hardware Config."
                    ToolStripStatusLabel1.BackColor = Color.Red
                    TabControl1.Enabled = False
                    Poll.Enabled = False
                    ReadOnce.Enabled = False
                    Append_Text1("Multiple WIN I2C Hardwares Connected to the Host" & vbCrLf)
                End If
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
#If 0 Then
                Win_I2C_Count = GetNumberOfDevices()
                If Win_I2C_Count = 0 Then
                    Hardware_Detected = 0
                    ToolStripStatusLabel1.Text = "WIN-I2C Hardware Not Detected"
                    ToolStripStatusLabel1.BackColor = Color.Red
                    TabControl1.Enabled = False
                    Poll.Enabled = False
                    ReadOnce.Enabled = False
                    Poll_Button_Latched = False
                    Poll.BackColor = Color.Transparent
                    Append_Text1("WIN I2C Hardware Disconnected from Host" & vbCrLf)
                End If
#End If
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



    Private Sub Button111_Click(sender As Object, e As EventArgs)
        'Set Default Calibration
        Write_Buf(0) = &H8
        Write_Buf(1) = &HFE
        Write_Buf(2) = &H0
        Write_Buf(3) = &H0
        Write_Buf(4) = &H0
        Write_Buf(5) = &H0
        Write_Buf(6) = &H0
        Write_Buf(7) = &H0
        Write_Buf(8) = &H0
        Write_Block_1(&HC9)

    End Sub




    Private Sub SaveAsCaliLog_Click(sender As System.Object, e As System.EventArgs)

    End Sub

    Private Sub Button64_Click(sender As System.Object, e As System.EventArgs)
        Write_Buf(0) = 1
        Write_Byte(&HCA)
    End Sub

    Private Sub Button45_Click(sender As System.Object, e As System.EventArgs)
        Write_Buf(0) = 0
        Write_Byte(&HCA)
    End Sub

#If 0 Then
    Private Sub SaveAsCaliLog_Click(sender As Object, e As EventArgs) Handles SaveAsCaliLog.Click
        Dim CaliLogPathStr As String
        Dim fileDialogSaveCaliLog As New SaveFileDialog
        Dim RichTextLines As Double
        fileDialogSaveCaliLog.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*"
        fileDialogSaveCaliLog.FilterIndex = 1
        fileDialogSaveCaliLog.RestoreDirectory = False
        'fileDialogSaveCaliLog.Title = "Save"
        'fileDialogSaveCaliLog.FileName = System.AppDomain.CurrentDomain.BaseDirectory
        'CaliLogPathStr = System.AppDomain.CurrentDomain.BaseDirectory & "\Calibration Log"
        'If Not My.Computer.FileSystem.DirectoryExists(CaliLogPathStr) Then
        'if not found the 'Calibration Log' folder, then creat it
        'My.Computer.FileSystem.CreateDirectory(CaliLogPathStr)
        'End If
        'CaliLogPathStr = System.AppDomain.CurrentDomain.BaseDirectory & "\Calibration Log\" & Format(DateTime.Now, "yyyyMMdd") & ".txt"
        'If Not My.Computer.FileSystem.FileExists(CaliLogPathStr) Then
        'if not found the file, then creat it
        'File.CreateText(CaliLogPathStr)
        'End If
        'RichTextLines = RichTextBox2.Lines.Length
        'Append_Text_Calibration(RichTextLines & vbCrLf)
        'For i = 0 To RichTextLines - 1
        'File.AppendAllText(CaliLogPathStr, RichTextBox2.Lines(i) & vbCrLf)
        'Next
        'Append_Text_Calibration("The data is saved to:" & CaliLogPathStr & vbCrLf)
        'File.WriteAllLines(CaliLogPathStr, strSp, Encoding.Default)
        If fileDialogSaveCaliLog.ShowDialog() = DialogResult.OK Then
            CaliLogPathStr = fileDialogSaveCaliLog.FileName
            File.CreateText(CaliLogPathStr)
            RichTextLines = RichTextBox2.Lines.Length
            For i = 0 To RichTextLines - 1
                File.AppendAllText(CaliLogPathStr, RichTextBox2.Lines(i) & vbCrLf)
            Next
        End If
    End Sub
#End If

    Private Sub RadioButton17_CheckedChanged(sender As System.Object, e As System.EventArgs)

    End Sub

    Private Sub ToolStripStatusLabel1_Click(sender As Object, e As EventArgs) Handles ToolStripStatusLabel1.Click

    End Sub

    Private Sub PictureBox76_Click(sender As Object, e As EventArgs)

    End Sub

    Private Sub PictureBox16_Click(sender As Object, e As EventArgs) Handles PictureBox16.Click

    End Sub

    Private Sub Label161_Click(sender As Object, e As EventArgs)

    End Sub

    Private Sub TextBox24_TextChanged(sender As Object, e As EventArgs)

    End Sub

    Private Sub NumericUpDown1_ValueChanged(sender As Object, e As EventArgs)

    End Sub

    Private Sub NumericUpDown16_ValueChanged(sender As Object, e As EventArgs) Handles NumericUpDown16.ValueChanged

    End Sub

    Private Sub PictureBox39_Click(sender As Object, e As EventArgs) Handles PictureBox39.Click

    End Sub

    Private Sub NumericUpDown2_ValueChanged(sender As Object, e As EventArgs) Handles NumericUpDown2.ValueChanged

    End Sub

    Private Sub NumericUpDown12_ValueChanged(sender As Object, e As EventArgs) Handles NumericUpDown12.ValueChanged

    End Sub

    Private Sub Label151_Click(sender As Object, e As EventArgs) Handles Label151.Click

    End Sub

    Private Sub DataGridView1_CellContentClick(sender As Object, e As DataGridViewCellEventArgs) Handles DataGridView1.CellContentClick

    End Sub

    Private Sub GroupBox4_Enter(sender As Object, e As EventArgs) Handles GroupBox4.Enter

    End Sub

    Private Sub Test_Click(sender As Object, e As EventArgs) Handles Test.Click
        If Test_Button_Latched = False Then
            Test_Button_Latched = True
            Test.BackColor = Color.GreenYellow
        Else
            Test_Button_Latched = False
            Test.BackColor = Color.Transparent
        End If
    End Sub

    Private Sub Button14_Click_1(sender As Object, e As EventArgs) Handles PICkitConnect.Click
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

    Private Sub ComboBox2_SelectedIndexChanged(sender As Object, e As EventArgs)

    End Sub

    Private Sub Button1_Click_1(sender As Object, e As EventArgs) Handles Button1.Click

    End Sub

    Private Sub Label18_Click(sender As Object, e As EventArgs) Handles Label18.Click

    End Sub

    Private Sub Button_MFR_Read_Click(sender As Object, e As EventArgs) Handles Button_MFR_Read.Click
        Append_Text1("Started to Read PMBus MFR Data........" & vbCrLf)
        Update_Pmbus_MFR(0)
    End Sub



    Private Sub MFR_EN_Click_1(sender As Object, e As EventArgs) Handles MFR_EN.Click

        Dim Addr_cmd_code As Byte = Convert.ToByte(TextBox10.Text, 16)
        Dim Data0 As Byte = Convert.ToByte(TextBox19.Text Mod 256, 16)
        Dim Data1 As Byte = Convert.ToByte(TextBox19.Text \ 256, 16)

        Write_Buf(0) = 6 'Convert.ToByte(TextBox32.Text, 10) ' Count
        Write_Buf(1) = 0 'Convert.ToByte(NumericUpDown15.Value, 10) 'Page
        Write_Buf(2) = &HD1 'Addr_cmd_code      'Command Code

        Write_Buf(3) = &H4C 'Data LSB
        Write_Buf(4) = &H69 'Data MSB

        Write_Buf(5) = &H6F 'Data LSB
        Write_Buf(6) = &H6E 'Data MSB

        Write_Block(&HD1)

    End Sub

    Private Sub Button13_Click_1(sender As Object, e As EventArgs) Handles Button13.Click
        Append_Text1("Started to Read PMBus Constant Data........" & vbCrLf)
        Update_Pmbus_Constant_1(0)
    End Sub

    Private Sub Button_LogRead_Click(sender As Object, e As EventArgs) Handles Button_LogRead.Click
        Append_Text1("Started to Read PMBus Event Log Data........" & vbCrLf)
        Update_Pmbus_Log(0)
    End Sub

    Private Sub Button_send_Query_Click(sender As Object, e As EventArgs) Handles Button_send_Query.Click

    End Sub

    Private Sub Label64_Click(sender As Object, e As EventArgs) Handles Label64.Click

    End Sub

    Private Sub Button_Clear_FRU_Click(sender As Object, e As EventArgs) Handles Button_Clear_FRU.Click

    End Sub

    Private Sub Button_FRU_File_Click(sender As Object, e As EventArgs) Handles Button_FRU_File.Click


        If Capture_Data = False Then
            Dim SaveFileDialog2 As New SaveFileDialog
            SaveFileDialog2.FileName = "FRU_Profile_xxx'C_xL_xxx.csv"
            SaveFileDialog2.InitialDirectory = "C:\Users\Chen Wuheng\Documents"

            If SaveFileDialog2.ShowDialog() = System.Windows.Forms.DialogResult.OK Then
                Dim bytes() As Byte
                bytes = Encoding.ASCII.GetBytes("Date & Time,T Ambient,T Hotspot,T Outlet,Fan Speed" & vbCrLf)

                FRU_File_Name = SaveFileDialog2.FileName
                My.Computer.FileSystem.WriteAllBytes(FRU_File_Name, bytes, False)
                Button_FRU_File.BackColor = Color.Green
                Data_Arr_Pntr = 0
                Capture_Data = True
                Capture_Data_Pntr = 0
            End If
        End If
    End Sub

    Private Sub Button_Clear_Faults_Click(sender As Object, e As EventArgs) Handles Button_Clear_Faults.Click
        Send_Byte(&H3)
    End Sub

    Private Sub Button_PSU_ON_Click(sender As Object, e As EventArgs) Handles Button_PSU_ON.Click
        'PSU ON
        Write_Buf(0) = &H0
        Write_Byte(&H1)
    End Sub

    Private Sub Button_PSU_OFF_Click(sender As Object, e As EventArgs) Handles Button_PSU_OFF.Click
        'PSU OFF
        Write_Buf(0) = &H80
        Write_Byte(&H1)
    End Sub
End Class
