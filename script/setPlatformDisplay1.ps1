# The script accepts four subsequent parameters, parameters are separated by space 
# Parameter1 First line. Train name
# Parameter2 second line eg. first destination
# Parameter3 third line eg. second destination
# Parameter4 departure time
# For german "umlauts" their representive must be used eg. "ae" instead of "ä"

# eg setPlatformDisplay1.ps1 IEC123 Muenich Salzburg 13:30

$COM3 = 'COM3'   #define the COM port used by Arduino

#Do not change anything past this line
# =========================
$Train=$args[0]
$Dest1=$args[1]
$Dest2=$args[2]
$Depart=$args[3]
$port= new-Object System.IO.Ports.SerialPort COM3,19200,None,8,one
$port.open()
$port.WriteLine('Train1' + '-L1 ' + $Train + '-L2 ' + $Dest1 + '-L3 '+ $Dest2 + '-L4 ' + $Depart)
$port.Close()