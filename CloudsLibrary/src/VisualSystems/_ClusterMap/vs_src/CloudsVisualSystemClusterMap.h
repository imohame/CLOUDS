
#pragma once

#include "CloudsVisualSystem.h"
#include "ofxGameCamera.h"
#include "Node.h"
#include "CloudsQuestion.h"

class CloudsVisualSystemClusterMap : public CloudsVisualSystem {
  public:
    
	//TODO: Change this to the name of your visual system
	//This determines your data path so name it at first!
	//ie getVisualSystemDataPath() uses this
    string getSystemName(){
		return "_ClusterMap";
	}

	//These methods let us add custom GUI parameters and respond to their events
    void selfSetupGui();
    void selfGuiEvent(ofxUIEventArgs &e);
    
	//Use system gui for global or logical settings, for exmpl
    void selfSetupSystemGui();
    void guiSystemEvent(ofxUIEventArgs &e);
    
	//use render gui for display settings, like changing colors
    void selfSetupRenderGui();
    void guiRenderEvent(ofxUIEventArgs &e);

	void selfSetupTimeline();
	
	// selfSetup is called when the visual system is first instantiated
	// This will be called during a "loading" screen, so any big images or
	// geometry should be loaded here
    void selfSetup();

	// selfBegin is called when the system is ready to be shown
	// this is a good time to prepare for transitions
	// but try to keep it light weight as to not cause stuttering
    void selfBegin();

	// selfPresetLoaded is called whenever a new preset is triggered
	// it'll be called right before selfBegin() and you may wish to
	// refresh anything that a preset may offset, such as stored colors or particles
	void selfPresetLoaded(string presetPath);
    
	//do things like ofRotate/ofTranslate here
	//any type of transformation that doesn't have to do with the camera
    void selfSceneTransformation();
	
	//normal update call
	void selfUpdate();

	// selfDraw draws in 3D using the default ofEasyCamera
	// you can change the camera by returning getCameraRef()
    void selfDraw();
		
    // draw any debug stuff here
	void selfDrawDebug();

	// or you can use selfDrawBackground to do 2D drawings that don't use the 3D camera
	void selfDrawBackground();

	// this is called when your system is no longer drawing.
	// Right after this selfUpdate() and selfDraw() won't be called any more
	void selfEnd();

	// this is called when you should clear all the memory and delet anything you made in setup
    void selfExit();

	//events are called when the system is active
	//Feel free to make things interactive for you, and for the user!
    void selfKeyPressed(ofKeyEventArgs & args);
    void selfKeyReleased(ofKeyEventArgs & args);
    
    void selfMouseDragged(ofMouseEventArgs& data);
    void selfMouseMoved(ofMouseEventArgs& data);
    void selfMousePressed(ofMouseEventArgs& data);
    void selfMouseReleased(ofMouseEventArgs& data);
	

    // if you use a custom camera to fly through the scene
	// you must implement this method for the transitions to work properly
	ofCamera& getCameraRef(){
		return cam;
	}

	//
//	ofCamera& getCameraRef(){
//		return CloudsVisualSystem::getCameraRef();
//	}
	
	void setQuestions(vector<CloudsClip>& questions);
	CloudsQuestion* getSelectedQuestion();
	
  protected:
    
    //  Your Stuff
    //
	
	ofxUISuperCanvas* generatorGui;
	ofxUISuperCanvas* displayGui;
	
	float seed; //read as int
	float heroNodes; //read as int
	float numIterations; //read as int
	float heroRadius;
	float heroRadiusVariance;
	float numBranches;
	float minDistance;
	float distanceRange;
	float stepSize;
	float minAttractRadius;
	float minRepelRadius;
	float minFuseRadius;
	
	float lineStartTime;
	float lineEndTime;
	float lineFadeVerts;
	float lineBlurAmount;
	float lineBlurFade;
	
	float maxAttractForce;
	float maxRepelForce;
	
	float maxTraverseDistance;
	float traverseNodeWeight;
	float traverseStepSize;
	
	float numSurvivingBranches;
	int numPointsAtReplicate;
	
	float replicatePointDistance;
	float replicatePointSize;
	float maxTraverseAngle;
	
	float nodePopLength;
	
	//taken from timeline
	float nodeBounce;
	float clusterNodeSize;
	float traversedNodeSize;
	float lineFocalDistance;
	float lineFocalRange;
	float lineWidth;
	float lineDissolve;
	float lineThickness;
	float lineAlpha;

	ofxGameCamera cam;

	ofxTLColorTrack* lineColor;
	ofxTLColorTrack* nodeColor;
	
	vector<Node*> nodes;
	vector<ofVec3f> fusePoints;
	ofVboMesh geometry;
	//fuzzy points
	ofVboMesh nodeCloudPoints;
	//for drawing the line
	ofVboMesh traversal;
	
	//ofxTLCurves* nodeBounce;
	
	
	//for drawing the node graphics
	vector<Node*> traversedNodes;
	map<ofIndexType,ofIndexType> traversalIndexToNodeIndex;
	ofVboMesh traversedNodePoints;
	
	void traverse();
	void generate();
	ofVec3f trailHead;
	ofShader billboard;
	ofShader lineAttenuate;
	
	ofShader gaussianBlur;
	string renderFolder;
	ofImage nodeSprite;
	ofImage nodeSpriteBasic;
	void loadShader();

	//TODO pick a better font renderer
	ofTrueTypeFont font;
	vector<CloudsQuestion> questions;
	CloudsQuestion* selectedQuestion;
	
};