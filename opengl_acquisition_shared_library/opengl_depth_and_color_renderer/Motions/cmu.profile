#BVH to TRI for SmartBody

#Main Body
JOINT_ASSOCIATION(head,JtUpperFaceParent)
JOINT_ASSOCIATION(abdomen,JtSpineB)
JOINT_ASSOCIATION(chest,JtNeckA)
JOINT_ASSOCIATION(neck,JtNeckB)
JOINT_ASSOCIATION(hip,JtSpineA)

#Right Foot
JOINT_ASSOCIATION(rhip,JtHipRt)
JOINT_ASSOCIATION(rknee,JtKneeRt)
JOINT_ASSOCIATION(rfoot,JtAnkleRt)

#Left Foot
JOINT_ASSOCIATION(lhip,JtHipLf)
JOINT_ASSOCIATION(lknee,JtKneeLf)
JOINT_ASSOCIATION(lfoot,JtAnkleLf)

#Right Hand
JOINT_ASSOCIATION(rcollar,JtClavicleRt)
JOINT_ASSOCIATION(rshoulder,JtShoulderRt)
JOINT_ASSOCIATION(relbow,JtElbowRt)
JOINT_ASSOCIATION(rhand,JtWristRt)
JOINT_ASSOCIATION(rthumb1,JtThumbARt)
JOINT_ASSOCIATION(rindex1,JtIndexARt)
JOINT_ASSOCIATION(rmid1,JtMiddleARt)
JOINT_ASSOCIATION(rring1,JtRingARt)
JOINT_ASSOCIATION(rpinky1,JtLittleARt)

#Left Hand 
JOINT_ASSOCIATION(lcollar,JtClavicleLf)
JOINT_ASSOCIATION(lshoulder,JtShoulderLf)
JOINT_ASSOCIATION(lelbow,JtElbowLf)
JOINT_ASSOCIATION(lhand,JtWristLf)
JOINT_ASSOCIATION(lthumb1,JtThumbALf)
JOINT_ASSOCIATION(lindex1,JtIndexALf)
JOINT_ASSOCIATION(lmid1,JtMiddleALf)
JOINT_ASSOCIATION(lring1,JtRingALf)
JOINT_ASSOCIATION(lpinky1,JtLittleALf)
 

JOINT_ROTATION_ORDER(neck,-x,-y,-z)
JOINT_OFFSET(neck,0,0,0)

JOINT_OFFSET(lshoulder,0,40,70)
JOINT_OFFSET(rshoulder,0,-40,-70)


