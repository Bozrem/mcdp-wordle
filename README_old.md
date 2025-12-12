# MCDP Wordle

## Project Summary
This project is an exploration of a reinforcement learning hybrid algorithm with an application to Wordle. It combines the best properties from both a Monte-Carlo and Dynamic Programming approach, creating an anytime algorithm that can do full explorations of the state space given enough time, which should also converge much quicker due to selecting based on an approximation.

The sections below outline the three primary deviances from a classic Monte-Carlo algorithm

## Algorithm
The algorithm generally follows closer to Monte-Carlo in the larger states, then tends more and more towards Dynamic Programming as we descend the tree and shrink the state space. 

Here are the three main deviances:

### Off-Policy Action Selection
Instead of the on-policy selection usually implemented with a Monte-Carlo approach, this algorithm will make use of a nice approximation for a guess's strength.

To evaluate a guess, simulate next state results for each of the remaining possible answers, then just calculate the average reduction in possible answers. In previous work on Wordle, I attempted to use this strategy for everything, but I found that it tended to diverge from optimal play when the state space got small.

Though this is slightly computationally expensive, it's much cheaper than the alternative of exploring those states and their own evaluations, as we would in DP. Furthermore, we can utilize some memoization to reuse these values in each episode, since they remain static regardless of the policy we're developing.

The percentages produced by this evaluation can be given to a softmax function to determine probabilities of selection. Alternatively, we could greedily select from them.

### Memoization of Monte-Carlo Results
An advantage of Wordle is that we can build a model-based algorithm. There is no point in a Monte-Carlo exploration of a path if we know we have already done it. The advantage of being model-based is that we know the Wordle answer, so we can decide if we have explored a path deterministically.

Say for example our episode has an answer word of "Snake"

If we are in an exploration with the only remaining possible answers being "Snake" and "Spoke", our approximation may suggest that we do "Snake". This hit's all correct, meaning it was a leaf node. We can now confidently mark that Q value in that state with the action of "Snake" and the answer as "Snake".

Now if we get to that same state again with an answer word of "Snake", even if our approximation suggests again that "Snake" is a better bet, we have already explored that path entirely, as evidenced by it's Q value being marked. Therefore, we remove it from the softmax function, and only make the guess of "Spoke", thereby ensuring we avoid duplicate exploration.

Furthermore, if we get to that state AGAIN, but we see that all possible guesses have a set Q value, then we can now just end by setting our own V value by the best Q.

### Dynamic Programming Evaluation on Smaller States
One of the issues that comes up with the memoization technique just described is how it impacts memory. It is very possible to have to keep a ton of tiny states in memory that barely get visited. Instead, this third variation reduces that footprint by switching entirely to a dynamic programming approach when the state size is small enough.

Take the previous episode descriptions as an example. In each of those, we had to do 3 entire episodes of exploration just to get the V for that tiny state.

Instead, when the state size is sufficiently small, we just do a full dynamic programming evaluation to compute V*. 

Though the previous solution would have gotten there eventually, there was a lot of wasted compute and memory usage in between. This method allows us to further avoid the slow down that Monte-Carlo introduces whilst now having to spend nearly as long doing the computation.

## Update Rule
In the exploration phase, we continue downwards until we hit a known V*, either through DP or when we know all children

We can then backpropagate our changes through the V and Q values. 
```python
class StateNode:
    def __init__(self, possible_words, initial=6.0):
        self.initial = initial
        self.num_outcomes = len(possible_words)
        self.q_values = {}

    def get_q(self, action):
        return 1.0 + (self.q_values[action].sum_v / self.num_outcomes)
        # 1.0 is the cost of a guess

class QEntry:
    def __init__(self, total_weight, initial):
        self.sum_v = total_weight * initial
        self.current_q = initial

def backprop_update(trajectory, v):
    if not trajectory: return
    parent, action, child = trajectory[-1]
    q_entry = parent.q_values[action]
    
    old_child_v = child.v
    
    # Calculate the change in sum
    q_entry.sum_v += (v - old_child_v)
    
    new_q = 1.0 + (q_entry.sum_v / parent.num_outcomes)
    q_entry.current_q = new_q

    if new_q < parent.v:
        parent.v = new_q
        backprop_update(trajectory[:-1], new_q)
```

## Runthrough
Example run to show how this algorithm works
### Exploration
<!-- TODO: Update these with real values -->
We begin the episode with the final word of "CHILD." Our first state is the full 2315 long answer list.

Our hueristic looks at the options and selects 



## Plan
An overall plan for how I can write this

### Components
- Actions - Guess Word Bitmap
    - Struct and some nice translation functions
    - Need to consider if it is computationally worthwhile to narrow this down as well
- State - Answer Word Bitmap
    - Pretty similar to Actions
- P - Transition Function
    - Highly optimized
    - Takes in:
        - Action - String
        - State
        - Answer
    - Returns a new State Bitmap
- Full DP

### Aspects to figure out
- Memoization techniques across multiple threads
- Managing V and Q updates across threads
    - May want to consider adding these to some sort of IO queue to process them separately
    - They are only used as a check, not required to have the correct number
        - May just repeat some computations occasionally, need to consider tradeoff

### Components to maybe Memoize
Need to experiment to see the memory vs computation tradeoff between all of these.
Ordered in most likely to be worthwhile to least.

The ones marked as DELETABLE mean that there points at which they will never be used again, so we can save memory

- Full V* Computations
    - Pretty much a requirement of the algorithm
    - DELETABLE
- Transition function
    - DELETABLE
- 