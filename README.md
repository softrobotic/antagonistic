# Tenma
### Python package for interfacing with TENMA 72-13360 Power Supply

**Install Package** by running the following command:
pip install git+https://github.com/softrobotic/tenma.git


**Import package** by running the following command:
from tenma import Tenma

**Workflow:**
# 1 - define power supply object
ps=Tenma() 

# 2 - perform serial handshake
ps.handshake('COM6')

# 3 - basic commands
ps.iset(0.5) # sets current
ps.vset(0.3) # sets voltage
ps.out(True) # sets output ON or OFF

# 4 - feedback commands
volts = ps.vget() # reads current voltage
amps = ps.iget() # reads current current
ohms = ps.rget() # reads current resistance value
watts = ps.pwrget() # reads current power value

# 5 - utilities
ps.impulse(volts, amps, period) # generates impulse of length "period"

# 6 - close connection
ps.close_connection() # closes serial connection
