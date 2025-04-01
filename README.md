
# Particle System을 활용한 효과 구현
 
##  List Up

### 참고영상
- https://www.youtube.com/shorts/AYhJBuNHC9k
- https://www.youtube.com/watch?v=f2TYIinAf2E

### 기능 구현
- 3D Particle Emitter 구현
	- Append Buffer의 카운터를 활용한 Indirect Draw Call 지원 
	- 시간 종속한 변수(Emission Rate / Life Time / Size)를 설정하여 파티클을 조절할 수 있도록 구현
- Particle 및 Emitter에 대한 Rendering 기능 구현
	- Motion Blur / Glow 등
- 입자 간 인력/척력 계산을 위한 Neighborhood Particle Search 알고리즘 구현
