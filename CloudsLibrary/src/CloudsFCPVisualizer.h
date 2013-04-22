#pragma once

#include "MSAPhysics2D.h"
#include "ofMain.h"
#include "CloudsFCPParser.h"
#include "CloudsEvents.h"

typedef struct {
    msa::physics::Particle2D* particle;
	string keyword;
} Node;

typedef struct {
	msa::physics::Spring2D* spring;
	vector<CloudsClip> clips;
} Edge;

class CloudsFCPVisualizer {
  public:
    CloudsFCPVisualizer();
	
    void setup(CloudsFCPParser& database);
    
    void setupPhysics();
    void createClusterPhysics();
    
    void updatePhysics();
    void drawPhysics();

    void exportForGraphviz();
    
    void drawGrid();
 
    map< pair<string,string>, int > sharedClips;
    
    int width,height;
    msa::physics::World2D physics;

    ofTrueTypeFont font;
    ofTrueTypeFont largFont;
	
	void addAllClipsWithAttraction();
	void addTagToPhysics(string tag);
	void clipChanged(CloudsStoryEventArgs& clips);
	
	//void addLinksToPhysics(CloudsClip& center, vector<CloudsClip>& connections, vector<float>& scores);
	
    void mousePressed(ofMouseEventArgs& args);
    void mouseMoved(ofMouseEventArgs& args);
    void mouseDragged(ofMouseEventArgs& args);
    void mouseReleased(ofMouseEventArgs& args);
    
    void keyPressed(ofKeyEventArgs& args);
    void keyReleased(ofKeyEventArgs& args);

	map<msa::physics::Particle2D*, CloudsClip> particleToClip;
	map<msa::physics::Particle2D*, string> particleName;
	map<string, msa::physics::Particle2D*> particlesByTag;
	
	map< pair<msa::physics::Particle2D*,msa::physics::Particle2D*>, msa::physics::Spring2D*> springs;
	map< msa::physics::Spring2D*, vector<string> > keywordsInSpring;
	set< msa::physics::Spring2D*> linkSprings;
	map< msa::physics::Particle2D*, int> particleBirthOrder;
	
	vector< CloudsClip > currentOptionClips;
	vector< msa::physics::Particle2D* > currentOptionParticles;
	vector< CloudsClip > pathByClip;
	vector< msa::physics::Particle2D* > pathByParticles;
	vector< msa::physics::Spring2D* > pathBySprings;
	map< msa::physics::Spring2D*, float > springScores;
	vector<string> clipLog;

	string currentTopic;
	
//	set<string> allTags;
//	vector<string, float> tagRadius;
//	vector<string, ofVec2f> tagCenter;
	
	msa::physics::Particle2D* centerNode;
	
	void windowResized(ofResizeEventArgs& args);
	
	void storyBegan(CloudsStoryEventArgs& clips);
	void clear();
	
	ofRectangle totalRectangle;
	float currentScale;
	ofVec2f currentTop;

	string selectionTitle;
	vector<CloudsClip> selectedClips;
	
	bool isEdgeSelected();
	bool isSelectedEdgeLink();
	CloudsClip getEdgeSource();
	CloudsClip getEdgeDestination();
	void linkedEdge();
	void unlinkEdge();
	
	bool getPathChanged();
	
	float springStrength;
	float restLength;
	float repulsionForce;
	float minRadius, maxRadius;

  protected:
	
	CloudsFCPParser* database;

	ofColor visitedColor;
	ofColor abandonedColor;
	ofColor hoverColor;
	ofColor selectedColor;
	ofColor nodeColor;
	ofColor currentNodeColor;
	ofColor lineColor;
	ofColor traceColor;
	
	ofVec2f graphPointForScreenPoint(ofVec2f screenPoint);
	ofVec2f screenPointForGraphPoint(ofVec2f graphPoint);
	
    msa::physics::Particle2D* selectedParticle;
    msa::physics::Particle2D* hoverParticle;
	msa::physics::Particle2D* particleNearPoint(ofVec2f point);
	
	msa::physics::Spring2D* selectedSpring;
	msa::physics::Spring2D* hoverSpring;
	msa::physics::Spring2D* springNearPoint(ofVec2f point);
	
	map< msa::physics::Particle2D*, ofVec2f > dampendPositions;
	
	float cursorRadius;
	float minMass, maxMass;
	float minScore, maxScore;
	float radiusForNode(float mass);

	bool pathChanged;
	bool hasParticle(string tagName);
};
