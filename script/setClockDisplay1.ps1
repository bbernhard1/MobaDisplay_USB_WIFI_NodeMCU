# The script accepts following parameters:
#'T' System   	... set the display to system time of the host computer, there MUST be a space after the 'T'
#'T' 16:31    	... set the display to to given time, in thsi case 16:31, there MUST be a space after the 'T'
#'S' 			... increment the time by by one minute

$COM3 = 'COM3'   #define the COM port used by Arduino

#Do not change anything past this line
# =========================
$Cmd=$args[0]
$Time=$args[1]
if ($Time -eq 'System') {
$Time = ([datetime]::now).tostring("HH:mm")
}
$port= new-Object System.IO.Ports.SerialPort $COM3,19200,None,8,one
$port.open()
$cmdString = 'Clock1' + ' ' + $Cmd + ' ' + $Time
$port.WriteLine($cmdString)
echo 'Payload: ' $cmdString
$port.Close()
