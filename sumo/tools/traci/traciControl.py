import socket, time, struct
from traciconstants import *



RESULTS = {0x00: "OK", 0x01: "Not implemented", 0xFF: "Error"}

class Phase:
    
    def __init__(self, duration, duration1, duration2, phaseDef):
        self._duration = duration
        self._duration1 = duration1
        self._duration2 = duration2
        self._phaseDef = phaseDef
        
    def write(self):
        print("Phase:")
        print("duration: ", self._duration)
        print("duration1: ", self._duration1)
        print("duration2: ", self._duration2)
        print("phaseDef: ", self._phaseDef)
        
class Logic:
    
    def __init__(self, subID, type, subParameter, currentPhaseIndex, phases):
        self._subID = subID
        self._type = type
        self._subParameter = subParameter
        self._currentPhaseIndex = currentPhaseIndex
        self._phases = phases
        
    def write(self):
        print("Logic:")
        print("subID:", self._subID)
        print("type:", self._type)
        print("subParameter:", self._subParameter)
        print("currentPhaseIndex:", self._currentPhaseIndex)
        for i in range(len(self._phases)):
            self._phases[i].write()

class Message:
    string = ""
    queue = []

_socket = socket.socket()
_message = Message()

class Storage:

    def __init__(self, content):
        self._content = content
        self._pos = 0

    def read(self, format):
        oldPos = self._pos
        self._pos += struct.calcsize(format)
        return struct.unpack(format, self._content[oldPos:self._pos])

    def readString(self):
        length = self.read("!i")[0]
        return self.read("!%ss" % length)[0]
    
    def readStringList(self):
        n = self.read("!i")[0]
        list = []
        for i in range(n):
            list.append(self.readString())
        return list     
    
    def ready(self):
        return self._pos < len(self._content) 


def _recvExact():
    result = ""
    while len(result) < 4:
        result += _socket.recv(4 - len(result))
    length = struct.unpack("!i", result)[0] - 4
    result = ""
    while len(result) < length:
        result += _socket.recv(length - len(result))
    return Storage(result)

def _sendExact():
    length = struct.pack("!i", len(_message.string)+4)
    _socket.send(length)
    _socket.send(_message.string)
    _message.string = ""
    result = _recvExact()
    for command in _message.queue:
        prefix = result.read("!BBB")
        err = result.readString()
        if prefix[2] or err:
            print prefix, RESULTS[prefix[2]], err
        elif prefix[1] != command:
            print "Error! Received answer %s for command %s." % (prefix[1], command)
        elif prefix[1] == CMD_STOP:
            length = result.read("!B")[0] - 1
            result.read("!%sx" % length)
    _message.queue = []
    return result

def readHead(result):
    result.read("!B")
    result.read("!B")
    result.read("!B")
    result.read("!B")
    result.read("!B")     # Length
    result.read("!B")     # Identifier
    result.read("!B")     # Variable
    result.readString()   # Induction Loop ID // Multi-Entry/Multi-Exit Detector ID // Traffic Light ID
    result.read("!B")     # Return type of the variable
    
def initTraCI(port):
    for wait in range(10):
        try:
            _socket.connect(("localhost", port))
            break
        except socket.error:
            time.sleep(wait)

            
def cmdSimulationStep(step):
    _message.queue.append(CMD_SIMSTEP)
    _message.string += struct.pack("!BBdB", 1+1+8+1, CMD_SIMSTEP, float(step), POSITION_ROADMAP)
    result = _sendExact()
    updates = []
    while result.ready():
        if result.read("!BB")[1] == CMD_MOVENODE: 
            updates.append((result.read("!idB")[0], result.readString(), result.read("!fB")[0]))
    return updates

def cmdGetInductionLoopVariable_idList(IndLoopID):
    _message.queue.append(CMD_GET_INDUCTIONLOOP_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(IndLoopID), CMD_GET_INDUCTIONLOOP_VARIABLE, ID_LIST, len(IndLoopID)) + IndLoopID

    result = _sendExact()
    readHead(result)
    return result.readStringList()           # Variable value 

def cmdGetInductionLoopVariable_lastStepVehicleNumber(IndLoopID):
    _message.queue.append(CMD_GET_INDUCTIONLOOP_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(IndLoopID), CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_NUMBER, len(IndLoopID)) + IndLoopID

    result = _sendExact()
    readHead(result)
    return result.read("!i")[0]    # Variable value

def cmdGetInductionLoopVariable_lastStepMeanSpeed(IndLoopID):
    _message.queue.append(CMD_GET_INDUCTIONLOOP_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(IndLoopID), CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_MEAN_SPEED, len(IndLoopID)) + IndLoopID

    result = _sendExact()
    readHead(result)
    return result.read("!f")[0]    # Variable value    

def cmdGetInductionLoopVariable_vehicleIds(IndLoopID):
    _message.queue.append(CMD_GET_INDUCTIONLOOP_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(IndLoopID), CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, len(IndLoopID)) + IndLoopID

    result = _sendExact()
    readHead(result)
    return result.readStringList()           # Variable value
  
  


def cmdGetMultiEntryExitDetectorVariable_idList(MultiEntryExitDetID):
    _message.queue.append(CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(MultiEntryExitDetID), CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE, ID_LIST, len(MultiEntryExitDetID)) + MultiEntryExitDetID

    result = _sendExact()
    readHead(result)
    return result.readStringList()           # Variable value 

def cmdGetMultiEntryExitDetectorVariable_lastStepVehicleNumber(MultiEntryExitDetID):
    _message.queue.append(CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(MultiEntryExitDetID), CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_NUMBER, len(MultiEntryExitDetID)) + MultiEntryExitDetID

    result = _sendExact()
    readHead(result)
    return result.read("!i")[0]    # Variable value

def cmdGetMultiEntryExitDetectorVariable_lastStepMeanSpeed(MultiEntryExitDetID):
    _message.queue.append(CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(MultiEntryExitDetID), CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE, LAST_STEP_MEAN_SPEED, len(MultiEntryExitDetID)) + MultiEntryExitDetID

    result = _sendExact()
    readHead(result)
    return result.read("!f")[0]    # Variable value    

def cmdGetMultiEntryExitDetectorVariable_vehicleIds(MultiEntryExitDetID):
    _message.queue.append(CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(MultiEntryExitDetID), CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, len(MultiEntryExitDetID)) + MultiEntryExitDetID

    result = _sendExact()
    readHead(result)
    return result.readStringList()           # Variable value

def cmdGetTrafficLightsVariable_idList(TLID):
    _message.queue.append(CMD_GET_TL_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(TLID), CMD_GET_TL_VARIABLE, ID_LIST, len(TLID)) + TLID

    result = _sendExact()
    readHead(result)
    return result.readStringList()           # Variable value 

def cmdGetTrafficLightsVariable_stateRYG(TLID):
    _message.queue.append(CMD_GET_TL_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(TLID), CMD_GET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE, len(TLID)) + TLID

    result = _sendExact()
    readHead(result)
    return result.readString()           # Variable value 

def cmdGetTrafficLightsVariable_statePBY(TLID):
    _message.queue.append(CMD_GET_TL_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(TLID), CMD_GET_TL_VARIABLE, TL_PHASE_BRAKE_YELLOW_STATE, len(TLID)) + TLID

    result = _sendExact()
    readHead(result)
    return result.readStringList()           # Variable value     

def cmdGetTrafficLightsVariable_completeDefinition(TLID):
    _message.queue.append(CMD_GET_TL_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(TLID), CMD_GET_TL_VARIABLE, TL_COMPLETE_DEFINITION, len(TLID)) + TLID

    result = _sendExact()
    readHead(result)
    
    result.read("!i")     # Length
    result.read("!B")     # Type of Number of logics
    nbLogics = result.read("!i")[0]    # Number of logics
    logics = []
    for i in range(nbLogics):
        result.read("!B")                       # Type of SubID
        subID = result.readString()
        result.read("!B")                       # Type of Type
        type = result.read("!i")[0]             # Type
        result.read("!B")                       # Type of SubParameter
        subParameter = result.read("!i")[0]     # SubParameter
        result.read("!B")                       # Type of Current phase index
        currentPhaseIndex = result.read("!i")[0]    # Current phase index
        result.read("!B")                       # Type of Number of phases
        nbPhases = result.read("!i")[0]         # Number of phases
        phases = []
        for j in range(nbPhases):
            result.read("!B")                   # Type of Duration
            duration = result.read("!i")[0]     # Duration
            result.read("!B")                   # Type of Duration1
            duration1 = result.read("!i")[0]    # Duration1
            result.read("!B")                   # Type of Duration2
            duration2 = result.read("!i")[0]    # Duration2
            result.read("!B")                   # Type of Phase Definition
            phaseDef = result.readStringList()  # Phase Definition
            
            phase = Phase(duration, duration1, duration2, phaseDef)
            phases.append(phase)
        
        logic = Logic(subID, type, subParameter, currentPhaseIndex, phases)
        logics.append(logic)
    
    return logics

# ! not tested yet !
def cmdGetTrafficLightsVariable_controlledLanes(TLID):
    _message.queue.append(CMD_GET_TL_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(TLID), CMD_GET_TL_VARIABLE, TL_CONTROLLED_LANES, len(TLID)) + TLID

    result = _sendExact()
    readHead(result)
    return result.readStringList()           # Variable value 

# ! not tested yet !
def cmdGetTrafficLightsVariable_controlledLinks(TLID):
    _message.queue.append(CMD_GET_TL_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(TLID), CMD_GET_TL_VARIABLE, TL_CONTROLLED_LINKS, len(TLID)) + TLID

    result = _sendExact()
    readHead(result)
    
    nbSignals = result.read("!i")     # Length
    signals = []
    for i in range(nbSignals):
        result.read("!B")                           # Type of Number of Controlled Links
        nbControlledLinks = result.read("!i")[0]    # Number of Controlled Links
        controlledLinks = []
        for j in range(nbControlledLinks):
            result.read("!B")                       # Type of Link j
            link = result.readStringList()          # Link j
            controlledLinks.append(link)
        
        signals.append(controlledLinks)
    
    return signals


def cmdChangeTrafficLightsVariable_statePBY(TLID, state):
    [phase, brake, yellow] = state
    _message.queue.append(CMD_SET_TL_VARIABLE)
    _message.string += struct.pack("!BBBi", 1+1+1+4+len(TLID)+1+4+4+len(phase)+4+len(brake)+4+len(yellow), CMD_SET_TL_VARIABLE, TL_PHASE_BRAKE_YELLOW_STATE, len(TLID)) + TLID
    _message.string += struct.pack("!Bi", TYPE_STRINGLIST, 3)
    _message.string += struct.pack("!i", len(phase)) + phase
    _message.string += struct.pack("!i", len(brake)) + brake
    _message.string += struct.pack("!i", len(yellow)) + yellow
    _sendExact()
    
def cmdStopNode(edge, objectID, pos=1., duration=10000.):
    _message.queue.append(CMD_STOP)
    _message.string += struct.pack("!BBiBi", 1+1+4+1+4+len(edge)+4+1+4+8, CMD_STOP, objectID, POSITION_ROADMAP, len(edge)) + edge
    _message.string += struct.pack("!fBfd", pos, 0, 1., duration)

def cmdChangeTarget(edge, objectID):
    _message.queue.append(CMD_CHANGETARGET)
    _message.string += struct.pack("!BBii", 1+1+4+4+len(edge), CMD_CHANGETARGET, objectID, len(edge)) + edge

def cmdClose():
    _message.queue.append(CMD_CLOSE)
    _message.string += struct.pack("!BB", 1+1, CMD_CLOSE)
    _sendExact()
    _socket.close()
