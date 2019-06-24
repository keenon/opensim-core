#ifndef MOCO_MOCOTRACK_H
#define MOCO_MOCOTRACK_H
/* -------------------------------------------------------------------------- *
 * OpenSim Moco: MocoTrack.h                                                  *
 * -------------------------------------------------------------------------- *
 * Copyright (c) 2019 Stanford University and the Authors                     *
 *                                                                            *
 * Author(s): Nicholas Bianco                                                 *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0          *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "osimMocoDLL.h"
#include "MocoStudy.h"
#include "MocoTool.h"

#include <OpenSim/Simulation/Model/Model.h>
#include "Common/TableProcessor.h"
#include "ModelOperators.h"
#include "Common/TRCFileAdapter.h"

namespace OpenSim {

class MocoWeightSet;
class MocoProblem;
class MocoTrajectory;

/// MocoTrack
/// ---------
/// This tool constructs problems in which state and/or marker trajectory data 
/// are tracked while solving for model kinematics and actuator controls. 
/// "Tracking" refers to cost terms that minimize the error between provided 
/// reference data and the associated model quantities (joint angles, joint 
/// velocities, marker positions, etc). 
///
/// State and marker tracking
/// -------------------------
/// State reference data (joint angles and velocities), marker reference data 
/// (x/y/z marker motion capture trajectories), or both may be provided via the
/// `states_reference` and `markers_reference` properties. For each set of 
/// reference data provided, a tracking cost term is added to the internal 
/// MocoProblem. 
///
/// setMarkersReference() only accepts a scalar TimeSeriesTable (either
/// directly or via a TableProcessor) containing x/y/x marker position values.
/// A TimeSeriesTableVec3 of markers is not accepted, but you may use the 
/// flatten() method to convert to a scalar TimeSeriesTable:
/// 
/// @code 
/// MocoTrack track;
/// 
/// TimeSeriesTableVec3 markers = TRCFileAdapter("marker_trajectories.trc");
/// TimeSeriesTable markersFlat(markers.flatten());
/// track.setMarkersReference(TableProcessor(markersFlat));
/// @endcode
/// 
/// If you wish to set the markers reference directly from a TRC file, use 
/// setMarkersReferenceFromTRC().
///
/// The `states_global_tracking_weight` and `markers_global_tracking_weight` 
/// properties apply a cost function weight to all tracking error associated 
/// provided reference data. The `states_weight_set` and `markers_weight_set`
/// properties give you finer control over the tracking costs, letting you set
/// weights for individual reference data tracking errors.
///
/// Control effort minimization
/// ---------------------------
/// By default, a MocoControlCost term is added to the underlying problem with
/// a weight of 0.001. Control effort terms often help smooth the problem
/// solution controls, and minimally affect the states tracking solution with a
/// sufficiently low weight. Use the `minimize_control_effort` and 
/// `control_effort_weight` properties to customize these settings.
///
/// Problem configuration options
/// -----------------------------
/// A time range that is compatible with all reference data may be provided. 
/// If no time range is set, the widest time range that is compatible will all 
/// reference data will be used. 
///     
/// If you would like to track joint velocities but only have joint angles in 
/// your states reference, enable the `track_reference_position_derivatives` 
/// property. When enabled, the provided position-level states reference data 
/// will be splined in order to compute derivatives. If some velocity-level 
/// information exists in the reference, this option will fill in the missing 
/// data with position derivatives and leave the existing velocity data intact. 
/// 
/// Since the data in the provided references may be altered by TableProcessor 
/// operations or appended to by `track_reference_position_derivatives`, the 
/// tracked data is printed to file in addition to the problem solution. The 
/// tracked data files have the following format 
/// "<tool_name>_tracked_<data_type>.sto" (e.g. "MocoTool_tracked_states.sto").
///
/// Default solver settings
/// -----------------------
/// - solver: MocoCasADiSolver
/// - dynamics_mode: explicit
/// - transcription_scheme: Hermite-Simpson
/// - optim_convergence_tolerance: 1e-2
/// - optim_constraint_tolerance: 1e-2
/// - optim_sparsity_detection: random
/// - optim_finite_difference_scheme: 'forward'
/// 
/// Basic example
/// -------------
/// Construct a tracking problem by setting property values and calling solve():
///
/// @code
/// MocoTrack track;
/// track.setName("states_tracking_with_reserves");
/// track.setModel(ModelProcessor("model_file.xml") |
///                ModOpAddExternalLoads() | 
///                ModOpAddReserves(1000));
/// track.setStatesReference("states_reference_file.sto");
/// track.set_track_reference_position_derivatives(true);
/// track.set_control_effort_weight(0.1);
/// MocoSolution solution = track.solve();
/// @endcode
///
/// Customizing a tracking problem
/// ------------------------------
/// If you wish to further customize the underlying MocoProblem before solving,
/// instead of calling solve(), call initialize() which returns a pre-configured
/// MocoStudy object:
///
/// @code
/// MocoTrack track;
/// track.setName("track_and_minimize_hip_compressive_force");
/// track.setModel(ModelProcessor("model_file.xml") |
///                ModOpAddExternalLoads());
/// track.setStatesReference("states_reference_file.sto");
///
/// MocoStudy moco = track.initialize();
///
/// auto& problem = moco.updProblem(); 
/// auto* hipForceCost = problem.addCost<MocoJointReactionCost>("hip_force");
/// hipForceCost->set_weight(10);
/// hipForceCost->setJointPath("/jointset/hip_r");
/// hipForceCost->setReactionMeasures({"force-y"});
///
/// auto& solver = moco.updSolver<MocoCasADiSolver>();
/// solver.set_dynamics_mode("implicit");
///
/// MocoSolution solution = moco.solve();
/// @endcode
///
/// @underdevelopment
class OSIMMOCO_API MocoTrack : public MocoTool {
OpenSim_DECLARE_CONCRETE_OBJECT(MocoTrack, MocoTool);

public:
    OpenSim_DECLARE_PROPERTY(states_reference, TableProcessor,
        "States reference data to be tracked. If provided, a "
        "MocoStateTrackingCost term is created and added to the internal "
        "MocoProblem. ");

    OpenSim_DECLARE_PROPERTY(states_global_tracking_weight, double,
        "The weight for the MocoStateTrackingCost that applies to tracking " 
        "errors for all states in the reference.");

    OpenSim_DECLARE_PROPERTY(states_weight_set, MocoWeightSet,
        "A set of tracking weights for individual state variables. The "
        "weight names should match the names of the column labels in the "
        "file associated with the 'states_reference' property.");

    OpenSim_DECLARE_PROPERTY(track_reference_position_derivatives, bool,
        "Option to track the derivative of position-level state reference "
        "data if no velocity-level state reference data was included in "
        "the `states_reference`. If velocity-reference reference data was "
        "provided for some coordinates but not others, this option will only "
        "apply to the coordinates without speed reference data. "
        "(default: false)");

    OpenSim_DECLARE_PROPERTY(markers_reference, TableProcessor,
        "Motion capture marker reference data to be tracked. The columns in "
        "the table should correspond to scalar x/y/z marker position "
        "values and the columns labels should have consistent suffixes "
        "appended to the model marker names. If provided, a "
        "MocoMarkerTrackingCost term is created and added to the internal "
        "MocoProblem.");

    OpenSim_DECLARE_PROPERTY(markers_global_tracking_weight, double,
        "The weight for the MocoMarkerTrackingCost that applies to tracking "
        "errors for all markers in the reference.");

    OpenSim_DECLARE_PROPERTY(markers_weight_set, MocoWeightSet,
        "A set of tracking weights for individual marker positions. The "
        "weight names should match the marker names in the "
        "file associated with the 'markers_reference' property.");

    OpenSim_DECLARE_PROPERTY(allow_unused_references, bool, 
        "Allow references to contain data not associated with any components "
        "in the model (such data would be ignored). Default: false.");

    OpenSim_DECLARE_PROPERTY(guess_file, std::string,
        "Path to a STO file containing a guess for the problem. The path can "
        "be absolute or relative to the setup file. If no file is provided, "
        "then a guess constructed from the variable bounds midpoints will be "
        "used.");

    OpenSim_DECLARE_PROPERTY(apply_tracked_states_to_guess, bool,
        "If a `states_reference` has been provided, use this setting to "
        "replace the states in the guess with the states reference data. This "
        "will override any guess information provided via `guess_file`.");

    OpenSim_DECLARE_PROPERTY(minimize_control_effort, bool,
        "Whether or not to minimize actuator control effort in the problem."
        "Default: true.");

    OpenSim_DECLARE_PROPERTY(control_effort_weight, double, 
        "The weight on the control effort minimization cost term, if it "
        "exists. Default: 0.001");

    MocoTrack() { constructProperties(); }

    void setStatesReference(TableProcessor states) {
        set_states_reference(std::move(states));
    }
    void setMarkersReference(TableProcessor markers) {
        set_markers_reference(std::move(markers));
    }
    void setMarkersReferenceFromTRC(const std::string& filename) {
        auto markers = TRCFileAdapter::read(filename);
        set_markers_reference(TimeSeriesTable(markers.flatten()));
    }

    MocoStudy initialize();
    MocoSolution solve(bool visualize = false);

private:
    Model m_model;
    TimeInfo m_timeInfo;

    void constructProperties();

    // Cost configuration methods.
    TimeSeriesTable configureStateTracking(MocoProblem& problem, Model& model);
    void configureMarkerTracking(MocoProblem& problem, Model& model);
    // Convenience method for applying data from a states reference to the 
    // problem guess.
    void applyStatesToGuess(const TimeSeriesTable& states, const Model& model,
            MocoTrajectory& guess);
};

} // namespace OpenSim

#endif // MOCO_MOCOTRACK_H
