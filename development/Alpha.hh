
#ifndef NEW_ALPHA_H
#define	NEW_ALPHA_H

#include <string>


class Alpha : public Transition
{
public:
	Alpha(std::string particleType, double transitionQValue, double intensity,
	 double finalLevelEnergy, int finalLevelAtomicMass, int finalLevelAtomicNumber);
	 
	~Alpha();
	
	
private:


};

#endif

