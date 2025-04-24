#include "FountainEmitter.h"
#include <iostream>
#include <algorithm>
using namespace std;
FountainEmitter::FountainEmitter(Drawable *_model, int number) : IntParticleEmitter(_model, number) {}

void FountainEmitter::updateParticles(float time, float dt, glm::vec3 camera_pos) {

    //This is for the fountain to slowly increase the number of its particles to the max amount
    //instead of shooting all the particles at once
    if (active_particles < number_of_particles) {
        int batch = 1;
        int limit = std::min(number_of_particles - active_particles, batch);
        for (int i = 0; i < limit; i++) {
            createNewParticle(active_particles);
            active_particles++;
        }
    }
    else {
        active_particles = number_of_particles; //In case we resized our ermitter to a smaller particle number
    }

    for(int i = 0; i < active_particles; i++){
        particleAttributes & particle = p_attributes[i];
        height_threshold = 20;
        
        if(particle.life <= 0.001){
            createNewParticle(i);
        }
        
        particle.life -= particle.life* 2.2*dt;
        particle.mass -= particle.mass * 2.2*dt;
    }
}

void FountainEmitter::createNewParticle(int index){
    particleAttributes & particle = p_attributes[index];

    particle.position = emitter_pos;

    particle.mass = 0.15f;
    particle.life = 0.15f;
}
