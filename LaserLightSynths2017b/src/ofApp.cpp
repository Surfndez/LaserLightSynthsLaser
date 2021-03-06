#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
	// print input ports to console
	midiIn.listPorts(); // via instance
	//ofxMidiIn::listPorts(); // via static as well
	
	// open port by number (you may need to change this)
	midiIn.openPort(0);
	//midiIn.openPort("IAC Pure Data In");	// by name
	//midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port
	
	// don't ignore sysex, timing, & active sense messages,
	// these are ignored by default
	midiIn.ignoreTypes(false, false, false);
	
	// add testApp as a listener
	midiIn.addListener(this);
	
	// print received messages to the console
	midiIn.setVerbose(true);
	
	
	ofEnableAntiAliasing();
	ofEnableSmoothing();
	ofSetFrameRate(60);

	screenWidth = 800;
	screenHeight = 800;
	
	guideImage.loadImage("img/MarischalGuide2.jpg");

	
	svgs.push_back(ofxSVG());
    svgs.back().load("img/laserlightsynthsAberdeen.svg");
    
	svgs.push_back(ofxSVG());
	svgs.back().load("img/MarischalCollegeGuide.svg");

	//projectorFbo.allocate(1024, 768, GL_RGB, 4);
	uiFbo.allocate(screenWidth, screenHeight, GL_RGB, 2); 

	//projectorFbo.begin();
	//ofSetColor(0);
	//ofRect(0,0,1024,768);
	//projectorFbo.end();
	
	ofSetColor(255);
	projectorPosition.set(screenWidth/5*2.14, screenHeight*4/5 * 0.99, screenWidth/5, screenHeight/5);
	
	laserManager.setup(screenWidth, screenHeight);
	//laserManager.connectToEtherdream();
	
	laserManager.renderLaserPreview = true;
	laserManager.showPostTransformPreview = true;

	int panelwidth = 200;
	
	ofxBaseGui::setDefaultWidth(panelwidth);
    ofxBaseGui::setDefaultHeight(16);
	ofxBaseGui::setDefaultSpacing(3);
    ofxBaseGui::setDefaultElementSpacing(3);
	ofxBaseGui::setDefaultElementIndentation(1);
	ofxBaseGui::setDefaultTextPadding(5);
	ofxBaseGui::setUseTTF(true);
	
	ofxBaseGui::loadFont("VeraMoIt.ttf",8, true);
	
	laserGui.setup("LaserManager", "laserSettings.xml");
	laserGui.setPosition(ofPoint(screenWidth+220 - panelwidth - 10,10));
	laserGui.setVisible(true);
	laserGui.add(&laserManager.connectButton);
	laserGui.add(laserManager.parameters);
	
	laserGui.load();
	panels.push_back(&laserGui);
	
	appGui.setup("App");
	appGui.setPosition(ofPoint(screenWidth+220 - panelwidth - 10,10));
	appGui.setVisible(false);
	appGui.add(pipeOrganData.editable.set("Pipe Organ editable", false));
	appGui.add(laserColumnPoints.set("laser column points", false));
	appGui.add(laserOrganPoints.set("laser organ points", false));
	appGui.add(showGuideImage.set("show guide image", true));
	
	
	redGui.setup("Laser Red", "laserred.xml");
	redGui.add(laserManager.redParams );
    greenGui.setup("Laser Green", "lasergreen.xml");
	greenGui.add(laserManager.greenParams );
    blueGui.setup("Laser Blue", "laserblue.xml");
	blueGui.add(laserManager.blueParams );
    redGui.load();
	greenGui.load();
	blueGui.load();
	redGui.setPosition(ofPoint(10 ,10));
	greenGui.setPosition(ofPoint(10 +panelwidth + 10,10));
	blueGui.setPosition(ofPoint(10 +panelwidth*2 + 20,10));
	
	
	//appGui.load();
	panels.push_back(&appGui);

	sync.tempo = 120;
	sync.startPosition = 0;//(60000/111) - 5; // start after 1 beat intro

	
	pipeOrganData.load();
	columnData.init();
	columnData.load();
	
	columnData.gui.setPosition(ofPoint(screenWidth+220 - panelwidth - 10,10));
	columnData.gui.setVisible(false);
	
	panels.push_back(&columnData.gui);
	
	//effectDomeLines.setDomeData(&domeData);
	effectParticles.setObjects(&pipeOrganData, &columnData);
    effectPipeOrganLines.setObjects(&pipeOrganData, &particleSystemManager);
	
	
	
	calculateScreenSizes();
	
}


//--------------------------------------------------------------
void ofApp::update(){
	
	float deltaTime = ofClamp(ofGetLastFrameTime(), 0, 0.2);
	ofSoundUpdate();
	laserManager.update();
	//if(music.getIsPlaying()) soundPositionMS = music.getPositionMS();

	//sync.update(soundPositionMS);
	
	//sync.update((float)midiFrameCount/24.0f * 60000.0f/sync.tempo );
	sync.update(midiFrameCount);
				
	//screenAnimation.update();
	
	particleSystemManager.update(deltaTime);
	effectLaserBeams.update(deltaTime);
	effectDomeLines.update(deltaTime);
	effectPipeOrganLines.update(deltaTime);
	
	
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);
	
	
	int numBands = 500;
	float vol = 0;
//	
//	
//	float * val = ofSoundGetSpectrum(numBands);
//	
//	for(int i = 0;i<numBands; i++) {
//		vol+=val[i];
//		
//	}

		
	uiFbo.begin();
	ofSetupScreenPerspective(screenWidth,screenHeight,50);

	ofSetColor(0);
	ofFill(); 
	ofRect(0,0,screenWidth, screenHeight);
	ofNoFill();
	ofSetColor(255);
	
	ofSetColor(100);
	if(showGuideImage) guideImage.draw(0,0,screenWidth,guideImage.height*(float)screenWidth/(float)guideImage.width) ;
	ofSetColor(255);
	//ofRect(projectorPosition.x-1, projectorPosition.y-1, projectorPosition.width+2, projectorPosition.height+2);
	//projectorFbo.draw(projectorPosition);
	
	pipeOrganData.draw();
	
	if((pipeOrganData.editable) && (laserOrganPoints)){
		vector<Pipe>& pipes = pipeOrganData.pipes;
		for(int i = 0; i< pipes.size(); i++) {
			Pipe& pipe = pipes[i];
			if((pipe.topDragging) || (pipe.bottomDragging)) {
				laserManager.addLaserLineEased(pipe.top, pipe.bottom, ofColor::white);
			} else {
				laserManager.addLaserLineEased(pipe.top, pipe.bottom, ofColor::cyan);
			} 
		}
	}
	ofSetupScreenPerspective(screenWidth,screenHeight,50);

	columnData.draw();
	
	
	if((columnData.editable) && (laserColumnPoints)){
		
		// laser the columns here
			
		for(int i = 0; i<columnData.numCols; i++) {
			for(float h = 0; h<1.00001; h+=0.2) {
				for(float r = 0; r<1.0001; r+=0.5) {
					laserManager.addLaserDot(columnData.getColumnPosition(i, h, r), ofColor :: white);
				}
			}
		}
	}
	
	
	for(int i = 0; i<notes.size(); i++) {
		ofxMidiMessage msg = notes[i];
		int columnindex = msg.channel-1;
		
		if(columnindex>=columnData.numCols) continue;
		
		ofPoint pos = columnData.getColumnPosition(columnindex, 1, ofMap(msg.pitch, 60,88, 0.2, 0.8, true));
       // cout << msg.pitch << endl;
		effectParticles.makeParticle(pos, columnindex);
		
	}
	
	notes.clear();
	
	ofDrawBitmapString(ofToString(round(ofGetFrameRate())), 0,10);
	
	
	
		
	ofNoFill();
	
	
	
	uiFbo.end();


	ofPopMatrix();
	
	uiFbo.begin();
	ofSetupScreenPerspective(screenWidth,screenHeight,50);

	
	// EFFECTS ---------------------------------------------
	
	drawEffects();
    

	if((currentSVG>=0) && (currentSVG<svgs.size())) {
		laserManager.addLaserSVG(svgs[currentSVG], ofPoint(screenWidth/2,295),ofPoint(1.365,1.365));
	
   
	}
	
	laserManager.draw();
	laserManager.renderLaserPath(ofRectangle(0,0,screenWidth, screenHeight), false);
	
	ofDrawBitmapString(sync.getString(), 1000,10);
	sync.draw(1100,10);
	
	uiFbo.end();
	uiFbo.draw(0,0);
	
	
	// GUI PANELS
	for(int i = 0; i<panels.size(); i++) {
		panels[i]->draw();
	}
	redGui.draw();
	greenGui.draw();
	blueGui.draw();
}

void ofApp :: drawEffects() {
	

	//drawSpirograph(ofPoint(640,250), 20,50,80, ofClamp(fmod(sync.currentBarFloat,2), 1,2)-1,ofClamp(fmod(sync.currentBarFloat,2), 0,1),fmod(sync.currentBarFloat,2) * 360);
	
		if(startSwirl>-1) {
		if((sync.currentBarFloat>=startSwirl) && (sync.currentBarFloat<startSwirl+16)) {
			
			
			float progress = ofMap(sync.currentBarFloat, startSwirl, startSwirl+16, 0,1);
			float brightness = 255;
			//
			if(sync.currentBarFloat<startSwirl) brightness = ofMap(sync.currentBarFloat, startSwirl-1,startSwirl,0,255);
			
			ofVec3f centre = laserManager.maskRectangle.getCenter();
			float numLines = 10;
			float zMin = -2000,
			zMax = 400;
			float spikeSpacing = 800;
			
			float zoffset = ofMap(fmod(sync.currentBarFloat, 0.25f), 0, 0.25, 0,spikeSpacing);
			float spinRotate = ofMap(fmod(sync.currentBarFloat, 4.0f), 0, 4, 0,-360);
			
			float twistAmount = ofMap(sync.currentBarFloat, startSwirl+8, startSwirl+16, 0, -480, true);
			
			float hueOffset = ofMap(fmod(sync.currentBarFloat, 0.5f), 0, 0.5, 0,255);
			
			
			
			float fade = 1;

			
			
			float sizeMin = 0;
			float sizeMax = 200;
			float spikeSize = 0.1;
			ofColor col ;
			
			
			float rotateMultiplier = ofMap(sync.currentBarFloat, startSwirl+14, startSwirl+16, 1, 0, true);
			
			float rotateXAmount = sin(sync.currentBarFloat*2.3) * 10 * rotateMultiplier;
			float rotateYAmount = sin(sync.currentBarFloat*1.9) * 15 *rotateMultiplier;
			
			
			ofPoint offset(40,40);
			offset.rotate(ofMap(fmod(sync.currentBarFloat, 4), 0, 4, 0,360), ofPoint(0,0,1));
			
			for(int i = 0; i<numLines; i++) {
				
				float angle = ofMap(i, 0, numLines, 0, 360) + spinRotate;
				
				poly.clear();
				
				for(float z = zMin-spikeSpacing; z< zMax+zoffset; z+=spikeSpacing) {
					
					float twist = ofMap(z+ zoffset, zMin-spikeSpacing, zMax, twistAmount, 0);
						ofPoint p;
					float size = ofMap(z+ zoffset, zMin-spikeSpacing, zMax, sizeMin, sizeMax);
					
					if(z>zMin) {
						p.set(size*(1+(spikeSize) ), 0, ofClamp(z+ zoffset, zMin, zMax));
						
						float overlap = 0;
						if(z+zoffset>zMax) {
							
							overlap = ((zMax- (z+zoffset)) / spikeSpacing);
							p.set(size*(1+(spikeSize*overlap) ), 0, ofClamp(z+ zoffset, zMin, zMax));
							
						}
						
						p.rotate(angle + (twist), ofPoint(0,0,1));
						
						//p+=offset;
						p.rotate(rotateXAmount,ofPoint(1,0,0));
						p.rotate(rotateYAmount,ofPoint(0,1,0));
						//p.rotate(xRotate, ofPoint(0,1,0));
						p+=centre;//+ offset;
						
						
						poly.addVertex(p);
					}
					
					if(z<zMax){
						p.set(size,0,ofClamp(z+ zoffset, zMin, zMax));
						
						p.rotate(angle+twist, ofPoint(0,0,1));
						//p+=offset;
						//p.rotate(rotateAmount,rotateAxis);
						p.rotate(rotateXAmount,ofPoint(1,0,0));
						p.rotate(rotateYAmount,ofPoint(0,1,0));
						
						p+=centre;// + offset;
						//p.rotate(xRotate, ofPoint(0,1,0));
						
						poly.addVertex(p);
					}
				}
				
				
				col.setHsb(fmod(ofMap(i, 0, numLines, 0, 255)+hueOffset,255.0f), 255,brightness);
				laserManager.addLaserPolyline(poly, new ColourSystem(col));
				
				
				
				
			}
			
		}
	}
	
	
	/*
	if((sync.currentBar>=24) && (sync.currentBar<28)) {
		// CHEVRONS
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(4);
	}
	if((sync.currentBar>=28) && (sync.currentBar<30)) {
		drawSpirograph(ofPoint(350,600), 20,50,80,ofMap(sync.currentBarFloat, 29, 30,0,1, true),ofMap(sync.currentBarFloat, 28, 29,0,1, true),
					   ofMap(sync.currentBarFloat, 28, 30, 180,0));
		effectDomeLines.setMode(0);
	}
	if((sync.currentBar>=30) && (sync.currentBar<32)) {
		drawSpirograph(ofPoint(1000,600), 20,80,60,ofMap(sync.currentBarFloat, 31, 32,0,1, true),ofMap(sync.currentBarFloat, 30, 31,0,1, true),
					   ofMap(sync.currentBarFloat, 28, 30, 180,0));
		effectDomeLines.setMode(0);
	}
	// SOLO
	if((sync.currentBar >= 32) && (sync.currentBar < 40)) {
		effectPipeOrganLines.setMode(2);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(0);
	}
	if((sync.currentBar >= 35) && (sync.currentBar < 36)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(0);
		if(sync.barTriggered) effectParticles.makeStarBurst();
	}
	if((sync.currentBar >= 40) && (sync.currentBar < 41)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(0);
	}
	if((sync.currentBarFloat > 41.5) && (sync.currentBar < 42)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(3);
	}// POST SOLO BUILD (DOME RINGS)
	if((sync.currentBar >= 42) && (sync.currentBar < 46)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(1);
	}
	// POST SOLO BUILD (DOME LINES)
	if((sync.currentBar >= 46) && (sync.currentBar < 50)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(2);
	}
	// MELLOW DROP OUT - WHITE LASER BEAMS
	if((sync.currentBar >= 50) && (sync.currentBar < 54)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 1;
		effectDomeLines.setMode(0);
	}
	// COLOURED LASER BEAMS
	if((sync.currentBar >= 54) && (sync.currentBar < 56)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 2;
		effectDomeLines.setMode(0);
	}
	// DROP OUT BEFORE OUTRO
	if((sync.currentBar >= 57) && (sync.currentBar < 59)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(3);
	}
	
	if((sync.currentBar>=59) && (sync.currentBar<74)) {
		if ((sync.barTriggered) && ((sync.currentBar==59)||(sync.currentBar == 63))) effectParticles.makeStarBurst();
			
		effectPipeOrganLines.setMode(3);
		effectLaserBeams.mode = 0;
		if((sync.currentBar>=67) && (sync.currentBar<73))  effectDomeLines.setMode(5);
		else effectDomeLines.setMode(0);
		if(sync.currentBar>=71) effectDomeLines.doSecondCircleRow = true;
	}
	//effectDomeLines.setMode(3;
	if((sync.currentBar >= 74) && (sync.currentBar < 75)) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(3);
		
	}
	if((sync.currentBar == 74) && (sync.currentBeat == 3) && (sync.eighthTriggered)){

		effectParticles.makeStarBurst(0.5);
		effectParticles.makeStarBurst(0.3);
	}
	
	if(sync.currentBar > 75) {
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(0);
		ofColor col;
		col.setHsb((ofGetElapsedTimeMillis()/4)%255, 255, 255);
		laserManager.addLaserText(" THANKS!", ofPoint(640,480), 10,col, true);
		
	}
	*/
	particleSystemManager.draw();
	effectLaserBeams.draw(laserManager, masterVolume);
	effectDomeLines.draw(sync, masterVolume, laserManager);
	
	// TODO add frequency - notes?
	effectPipeOrganLines.draw(sync, masterVolume, laserManager, 10);


}

void ofApp ::  drawSpirograph(ofPoint pos, int numrevolutions, float smallradius, float largeradius, float start, float end, float rotation) {
	
	poly.clear();
	ColourSystemGradient* grad = new ColourSystemGradient();
	
	int totaldivisions =360;
	//int numrevolutions = 20;
	//float smallradius = 50;
	//float largeradius = 200;
	ofPoint p;
	for(int i = totaldivisions*start; i<=totaldivisions*end; i++) {
		float angle1 = ofMap(i, 0, totaldivisions, 0, 360);
		float angle2 = ofMap(i, 0, totaldivisions, 0, 360*numrevolutions);
		
		float sr = smallradius;
//		if((i/10)<volumes.size())
//			sr*= ofMap(volumes[i/10],0,1,0.8,1.1,true);
//		
		sr*= ofMap(fmod(sync.beatPulse + ((float)i*0.0053),1), 0, 1, 0.6,1);
		
		p.set(sr,0,0);
		
		p.rotate(angle2, ofPoint(0,0,1));
		p.x+=largeradius;
		p.rotate(angle1+rotation, ofPoint(0,0,1));
		
		p+=pos;
		//p.z += i;
		
		poly.addVertex(p);
		int hue = ((int)ofMap(i,0,totaldivisions, 0, 255))%255;
		ofColor c;
		
		float brightness = 255;
		float fadedistance = 5; 
		
		if(i<totaldivisions*start+20) {
			brightness = ofMap(i, totaldivisions*start,totaldivisions*start+20, 0,255 );
			
		}

		if(i>totaldivisions*end-20) {
			brightness = ofMap(i, totaldivisions*end,totaldivisions*end-20, 0,255 );
			
		}
		c.setHsb(hue, 255,brightness);
		grad->addColourStop(c, ofMap(i,totaldivisions*start,totaldivisions*end, 0, 1));
		
		
		
	}
	//poly.draw();
	laserManager.addLaserPolyline(poly, grad);
	
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
	if(key == 'f') ofToggleFullscreen();
	if(key == '\t') {
		
		int activePanelIndex = -1; 
		for(int i =0; i<panels.size(); i++) {
			if(panels[i]->getVisible()) activePanelIndex = i;
			panels[i]->setVisible(false);
			
		}
		if(activePanelIndex ==-1) {
			panels[0]->setVisible(true);
		} else if(activePanelIndex<panels.size()-1) {
			panels[activePanelIndex+1]->setVisible(true);
		}
		
		redGui.setVisible(laserGui.getVisible());
		greenGui.setVisible(laserGui.getVisible());
		blueGui.setVisible(laserGui.getVisible());
			
	}
	if(key == 'w') laserManager.showWarpPoints = !laserManager.showWarpPoints;
	//if(key == ' ') previewProjector = !previewProjector;
	if(key == OF_KEY_DOWN) {
		//music.setPosition(0);
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(0);

		//music.play();
	}
	if(key == OF_KEY_LEFT) {
		//soundPositionMS = sync.getMSForBarNumber(sync.currentBar-1);
		//music.setPositionMS(soundPositionMS);
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(0);
	} else if(key == OF_KEY_RIGHT) {
	//	soundPositionMS = sync.getMSForBarNumber(sync.currentBar+1);
		//music.setPositionMS(soundPositionMS);
		effectPipeOrganLines.setMode(0);
		effectLaserBeams.mode = 0;
		effectDomeLines.setMode(0);
	}
	if(key =='l') laserManager.showLaserPath = !laserManager.showLaserPath;
		
		
	
	if(key == 'p') {
//		if(music.getIsPlaying()) {
//			
//			music.stop();
//		} else {
//
//			music.play();
//			music.setPositionMS(soundPositionMS);
//			effectPipeOrganLines.setMode(0);
//			effectLaserBeams.mode = 0;
//			effectDomeLines.setMode(0);
//		}
	
	}
	if(key=='c') {
		currentSVG++;
		if(currentSVG>=svgs.size()) currentSVG = -1;
	}
	 

	if(key == '1') {
		effectParticles.mode = 1;
		
		//effectLaserBeams.mode = 0;
		effectPipeOrganLines.setMode(1);
		//effectDomeLines.setMode(1);
	}
	if(key == '2') {
		effectParticles.mode = 2;
//		effectLaserBeams.mode = 0;
		effectPipeOrganLines.setMode(2);
//		effectDomeLines.setMode(2);
	}
	if(key == '3') {
		effectParticles.mode = 3;
//		effectLaserBeams.mode = 1;
		effectPipeOrganLines.setMode(3);
//		effectDomeLines.setMode(0);
	}
	if(key == '4') {
		effectParticles.mode = 4;
//		effectLaserBeams.mode = 2;
//		effectPipeOrganLines.setMode(0);
//		effectDomeLines.setMode(0);
	}
	if(key == '5') {
//		effectLaserBeams.mode = 0;
//		effectPipeOrganLines.setMode(1);
//		effectDomeLines.setMode(0);
	}
	if(key == '6') {
//		effectLaserBeams.mode = 0;
//		effectPipeOrganLines.setMode(2);
//		effectDomeLines.setMode(0);
	}
	if(key == '7') {
	
//		effectLaserBeams.mode = 0;
//		effectPipeOrganLines.setMode(3);
//		effectDomeLines.setMode(0);
	}
	
	if(key == '0') {
		effectParticles.mode = 0;
		effectLaserBeams.mode = 0;
		effectPipeOrganLines.setMode(0);
		effectDomeLines.setMode(0);
	}
	
	if(key == 's') {
		effectParticles.makeStarBurst(0.5);
	}
	if(key == 'a') {
		startSwirl = sync.currentBar+1;
	}
	

}


//--------------------------------------------------------------
void ofApp::windowResized(int width, int height){
	
	
	calculateScreenSizes();
}


void ofApp::calculateScreenSizes(){
    int monitorCount;
	
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
	
	//cout << "RESIZE" << " " << ofGetWindowMode()<< endl;
   // screens.clear();
    
    int leftMost = 0,
	topMost = 0;
    
	vector<ofRectangle> screens;
	
    for(int i = 0; i < monitorCount; i++){
		
		ofRectangle screen;
		
		int x=0,
		y=0,
		w=0,
		h=0;
		
        glfwGetMonitorPos(monitors[i],&x,&y);
        const GLFWvidmode * desktopMode = glfwGetVideoMode(monitors[i]);
		screen.x = x;
		screen.y = y;
        screen.width = desktopMode->width;
        screen.height = desktopMode->height;
        
        screens.push_back(screen);
        cout << i << " " << screen << endl;
		if( leftMost > screen.x ) leftMost = screen.x;
		if( topMost > screen.y ) topMost = screen.y;
		
    }
    
    for(int i = 0; i < monitorCount; i++){
	//	screens[i].x -= leftMost;
	//	screens[i].y -= topMost;
		
    }
    
	//std::sort( screens.begin(), screens.end(), screenSort );
	
	//uiScreenRect = screens.back();
	
	
	if(screens.size()>1) {
		secondScreenRect = screens[1];
	} else {
		secondScreenRect.set(screens[0].getRight(), screens[0].getTop(), 1024,768);
	}
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

	
	
}


//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int numChannels){
			
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	
	pipeOrganData.mouseDragged(x,y); 
	
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	cout << "MOUSE : " << x << " " << y << endl;
	pipeOrganData.mousePressed(x,y); 
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	pipeOrganData.mouseReleased(x,y);

}


//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

void ofApp::exit() {
	redGui.save();
	greenGui.save();
	blueGui.save();
	
	laserGui.save();
	pipeOrganData.save();
	columnData.save();
	laserManager.warp.saveSettings();

}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
	
	
	if ((msg.status == MIDI_START) || (msg.status == MIDI_CONTINUE)) {
		midiFrameCount = 0;
		
		// set playing to be true?
		
	} else if (msg.status == MIDI_SONG_POS_POINTER) {
		midiFrameCount = msg.value *24;
		
		
	} else if (msg.status == MIDI_TIME_CLOCK) {
		midiFrameCount++;
	} else if (msg.status == MIDI_NOTE_ON) {
	
		notes.push_back(msg);
		
	}
	
}

