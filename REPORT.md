# Monte-Carlo and Dynamic Programming Hybrid Algorithm for Wordle (MCDP)

## Introduction
Wordle is a complicated game from a computational perspective. At each guess, there are nearly 13,000 actions / words to guess, and the result depends on one of up to 2300 answers. As such, it's difficult to be able to have a true "solution" to the game that does not take ages to solve. An approach taken by an [MIT paper](https://auction-upload-files.s3.amazonaws.com/Wordle_Paper_Final.pdf) takes the pure DP approach. According to their paper, they made an optimized DP algorithm, and it "took days to solve [...] parallelized across a 64-core computer." Though they did get an answer, this isn't reasonable, as if the words get updated (as they have since that paper), that's another few days to recompute. 

Another approach that has been used (though I haven't had a chance to read any papers on it) has been a pure Monte-Carlo approach. The issue that the MIT paper points out is that a MC approach can be inefficient, but you also can't know that you actually have a perfectly optimal solution.

This algorithm aims to take the best features of both the Monte-Carlo approach and the DP approach, and combine them. Notably, it ends up with the following properties:
### Anytime Algorithm
Due to not relying on knowing all children, MCDP gets the Monte-Carlo advantage of being able to stop at any point and still have a half decent answer
### Terminating
A huge disadvantage of Monte-Carlo is that it never actually terminates, so you can never provably know you have the correct answer. MCDP gets the advantage from DP of being able to know when it has actually solved a state, and can propagate changes up the tree all the way until it can solve the root node.
### Fast, Unbiased Convergence
Instead of the MC approach of using known values to select actions in exploration, this MCDP implementation uses a heuristic that is highly accurate at large states, and is thus able to select good actions right from the start. In graphs, this would be a comparison between a linear decrease in cost from a normal anytime DP and an exponential decay, only at the cost of some extra compute overhead.

## Markov Decision Process
### States
The initial naive approach to a Wordle state (and one I took in previous work) is to say that a state is every guess and color results previously taken. Though this is an accurate representation, it is HUGELY inefficient. Instead, it turns out that we can perfectly emulate this to be simply which answers are still possible given previous results in the game. In code, I represent this with the state_bitmap type, which simply says for each possible answer in the game, is it still possible given the history. 

Representing all the history in a single bitmap is both a much more computationally efficient way of representing the state, and also makes it clear that this is a Markov state, in that a single bitmap encodes the whole game's history.

### Actions
In Wordle, the actions are simply every word you are allowed to guess. Unfortunately, this is a huge set of nearly 13,000 words, making it extraordinarily difficult for normal Monte-Carlo approaches. To counter this, we use [Action Pruning](###action-pruning).

### Transition
Transitions are handled in the wordle.c file in src. My approach is that at the start, I precompute all actual guesses in Wordle in terms of guess and answer. Though this could be done in a more Dynamic Programming memoization fashion at runtime, it's much more efficient to have this whole table, as it allows vectorizing transitions.

For full transitions, we have to take a state (list of every possible remaining answer), a guess / action, and the actual answer to use. This is a slight deviation from the usual definition of a transition, because the actual answer we're using isn't encoded into the state. This is because by deciding what answer we would like to follow at each guess, we can try to only choose answers for which we have never done the computation, helping accelerate fully solving that state. Unfortunately, I have not yet identified a way to do this that doesn't take a significant amount of memory and compute.

In the transition, all we do is generate the "true pattern" that we would get with that guess and answer, then compare that to the imaginary patterns we would get with each of the other remaining answers. You can imagine this as keeping a whiteboard going with all the words you think it could be, then you make a guess and find out a certain letter is green, then going through and eliminating everything which that letter would not have been green had it been the answer.

### Reward and Discount Factor
The way I handle rewards is slightly different from the usual approach in RL. MCDP sees the value of any state or state-action pair as more of a cost. Given some state (remaining possibilities), the V represents the expected value of guesses that it will take us given our best known policy. This makes us not need a discount factor, and the reward at each transition is essentially to decrease the cost by one. If a child's V is 3, the parent has to take one guess to get to the child, and it's V is thus 4. 


## Algorithm
### Exploration
The algorithm can largely be broken into two phases in exploration. Firstly, it uses the heuristic to guide actions in a normal Monte-Carlo decent through a few states. Second, when the system detects that a state is sufficiently small, it can switch to a DP approach, which can get us a V* value for a state really quickly when they're small enough, with the advantage of avoiding many more explorations down there to learn it all.

### Updating
To actually be able to track convergence, MCDP uses a pessimistic update rule. Here is the update flow:
1. Calculate delta from the new V and old V
If the child state was a V of 4, and it dropped to a V of 3 (we use smaller numbers as a better V, so think of it more of a cost), our delta is -1
2. New Q = Old Q + (delta / number of children)
In the example above, if Q broke off into 5 different children, and our old Q was 4, our new Q would be 3.8

Having this rolling average has mathematical equivalence to recalculating Q based off the V values of each child, but can instead do it in O(1) time.

### Action Pruning
A huge optimization that can be applied for Wordle specifically is the ability to prune down our action space. If we imagine that we had previously guessed a word that had a "P" in it and got a gray, then the guess of "YUIOP" is guaranteed to be worse than a guess of "YUIOL". An easy way to do this in code is to simply look at what each guess ends up with in possible answers, and anytime that a guess has either no decrease in answers (useless guess), or is a strict subset of another guess (duplicate information), then we can eliminate it from the action set and save ourselves a lot of compute down the road.

## Architecture
The flow through the source code is designed as follows
1. Main.c
This is where we do all the initial input parsing and setup. After that, it enters a parallel for loop to run a batch of episodes from episode.c before making a checkpoint (using memory.c)
2. Episode.c
This is where a lot of the core components to the algorithm are. It has the overall episode runner, which does a full [exploration](###exploration) before applying the [update](###updating). It also has the function for a pure DP exploration, and the expansion engine for when we explore to an uninitialized state.

## Algorithm Drawbacks
Though I'm still working on reducing it, this is a very memory and compute heavy algorithm. I'm designing it to be run on the Lotus cluster, and I'll likely require most of the 1.5TB of memory on each node. Hopefully, with the right optimizations, I'll be able to make a full comparison to the convergence and solving time between MCDP and pure DP

## Project Status
Though I did fully think out the algorithm, I have not had the time or ability to get it implemented yet. This is due to the sheer computational complexity of the algorithm requiring heavy optimization. I would have liked to have been able to get some initial results, but I wasn't able to get enough of the foundational architecture done in time to be able to get any real data.

I really love this project, and it was something I was planning even before it became my RL final project, so I may have just bit off a bit more than I can chew for this. I have the algorithm all worked out, so I hope I was able to talk about it well and present it well, but getting the full implementation is going to be mostly over break. I think it has a lot of potential, and if I can get real results I may look at making a small paper on it.

Sorry for not getting as far as we had hoped, but I spent a lot of time working through the idea, so I hope it's interesting.

## Future Algorithm Expansion
Though there are definitely some aspects I need to work on generalizing, this algorithm seems really interesting if I can get it working. It is heavily model-based, and it requires an optimal substructure like DP. However, I think that it could theoretically be applied as an anytime algorithm drop in to some things that currently use DP.
