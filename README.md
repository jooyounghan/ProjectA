
# Particle System을 활용한 효과 구현
 
##  List Up

### 참고영상
- https://www.youtube.com/shorts/AYhJBuNHC9k
- https://www.youtube.com/watch?v=f2TYIinAf2E

### 기능 구현
- 3D Particle Manager 구현
	- 여러 파티클 Emitter을 하나의 Manager가 관리하고, 각 Emitter가 생성 / 삭제되는 과정에서 발생할 수 있는 GPU 오버헤드(메모리 할당 등)를 줄이기 위하여 Particle을 Pooling하는 방식으로 구현 
	- Append Buffer의 카운터를 활용하여 Indirect Draw Call을 지원하여 렌더링 파이프라인에서 발생할 수 있는 오버헤드 최소화 
	- 시간에 종속한 변수(Emission Rate / Life Time / Size)를 사용자가 변경할 수 있도록 구현
- Particle 및 Emitter에 대한 Rendering 기능 구현
	- Motion Blur / Glow 등으로 사실적인 렌더링 구현
- 입자 간 인력/척력이 있을 경우, 전체 입자에 대해 연산하지 않고 영향을 받을 수 있는 입자를 찾는 Neighborhood Particle Search 알고리즘 구현
