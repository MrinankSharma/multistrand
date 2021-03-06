
## Often recurring experimental setups
from multistrand.objects import Complex, Domain, Strand, StopCondition
from multistrand.options import Options
from setuptools.dist import sequence
from reportlab.platypus.para import lengthSequence


def setBoltzmann(complexIn, trials):
    
    complexIn.boltzmann_count = trials
    complexIn.boltzmann_sample = True


# easy handle for options creation
def standardOptions(simMode = Options.firstStep, tempIn = 25.0, trials=10, timeOut= 0.1):
    
    output = Options(simulation_mode=simMode,
                      rate_method=Options.metropolis,
                      num_simulations=trials,
                      simulation_time=timeOut,
                      temperature=tempIn
                      )

    output.DNA23Metropolis()
    
    return output



def hybridization(options, mySeq, myTrials=0, doFirstPassage=False):
                
    # Using domain representation makes it easier to write secondary structures.
    onedomain = Domain(name="itall", sequence=mySeq)
    top = Strand(name="top", domains=[onedomain])
    bot = top.C
    
    # Note that the structure is specified to be single stranded, but this will be over-ridden when Boltzmann sampling is turned on.
    startTop = Complex(strands=[top], structure=".")
    startBot = Complex(strands=[bot], structure=".")
    
    
    # Turns Boltzmann sampling on for this complex and also does sampling more efficiently by sampling 'trials' states.
    if(myTrials > 0):
        setBoltzmann(startTop, myTrials)
        setBoltzmann(startBot, myTrials)
    
    # Stop when the exact full duplex is achieved.
    success_complex = Complex(strands=[top, bot], structure="(+)")
    stopSuccess = StopCondition(Options.STR_SUCCESS, [(success_complex, Options.exactMacrostate, 0)])
    
    # Declare the simulation unproductive if the strands become single-stranded again.
    failed_complex = Complex(strands=[top], structure=".")
    stopFailed = StopCondition(Options.STR_FAILURE, [(failed_complex, Options.dissocMacrostate, 0)])
    
    options.start_state = [startTop, startBot]

    # Point the options to the right objects
    if not doFirstPassage:
    
        options.stop_conditions = [stopSuccess, stopFailed]           
    
    else :

        options.stop_conditions = [stopSuccess]         
        
        

 