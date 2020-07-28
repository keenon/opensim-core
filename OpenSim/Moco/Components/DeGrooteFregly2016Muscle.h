#ifndef OPENSIM_DEGROOTEFREGLY2016MUSCLE_H
#define OPENSIM_DEGROOTEFREGLY2016MUSCLE_H
/* -------------------------------------------------------------------------- *
 * OpenSim: DeGrooteFregly2016Muscle.h                                        *
 * -------------------------------------------------------------------------- *
 * Copyright (c) 2017 Stanford University and the Authors                     *
 *                                                                            *
 * Author(s): Christopher Dembia                                              *
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

#include <OpenSim/Moco/MocoUtilities.h>
#include <OpenSim/Moco/osimMocoDLL.h>

#include <OpenSim/Common/DataTable.h>
#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/Muscle.h>

namespace OpenSim {

// TODO avoid checking ignore_tendon_compliance() in each function;
//       might be slow.
// TODO prohibit fiber length from going below 0.2.

/** This muscle model was published in De Groote et al. 2016.
The parameters of the active force-length and force-velocity curves have
been slightly modified from what was published to ensure the curves go
through key points:
  - Active force-length curve goes through (1, 1).
  - Force-velocity curve goes through (-1, 0) and (0, 1).
The default tendon force curve parameters are modified from that in De
Groote et al., 2016: the curve is parameterized by the strain at 1 norm
force (rather than "kT"), and the default value for this parameter is
0.049 (same as in TendonForceLengthCurve) rather than 0.0474.

The fiber damping helps with numerically solving for fiber velocity at low
activations or with low force-length multipliers, and is likely to be more
useful with explicit fiber dynamics than implicit fiber dynamics (when
support for fiber dynamics is added).

This class supports tendon compliance dynamics in both explicit and implicit
form. Both forms of the dynamics use normalized tendon force as the state
variable (rather than the typical fiber length state). The explicit form is
handled through the usual Component dynamics interface. The implicit form
introduces an additional discrete and cache SimTK::State variable for the
derivative of normalized tendon force and muscle-tendon equilibrium residual
respectively. The implicit form is only for use with solvers that support
implicit dynamics (i.e. Moco) and cannot be used to perform a time-stepping
forward simulation with Manager; use explicit mode for time-stepping.

@note Normalized tendon force is bounded in the range [0, 5] in this class.
      The methods getMinNormalizedTendonForce() and
      getMaxNormalizedTendonForce() are available to access these bounds for
      use in custom solvers.

@underdevelopment

@section departures Departures from the Muscle base class

The documentation for Muscle::MuscleLengthInfo states that the
optimalFiberLength of a muscle is also its resting length, but this is not
true for this muscle: there is a non-zero passive fiber force at the
optimal fiber length.

In the Muscle class, setIgnoreTendonCompliance() and
setIngoreActivationDynamics() control modeling options, meaning these
settings could theoretically be changed. However, for this class, the
modeling option is ignored and the values of the ignore_tendon_compliance
and ignore_activation_dynamics properties are used directly.

De Groote, F., Kinney, A. L., Rao, A. V., & Fregly, B. J. (2016). Evaluation
of Direct Collocation Optimal Control Problem Formulations for Solving the
Muscle Redundancy Problem. Annals of Biomedical Engineering, 44(10), 1–15.
http://doi.org/10.1007/s10439-016-1591-9 */
class OSIMMOCO_API DeGrooteFregly2016Muscle : public Muscle {
    OpenSim_DECLARE_CONCRETE_OBJECT(DeGrooteFregly2016Muscle, Muscle);

public:
    OpenSim_DECLARE_PROPERTY(activation_time_constant, double,
            "Smaller value means activation can change more rapidly (units: "
            "seconds).");
    OpenSim_DECLARE_PROPERTY(deactivation_time_constant, double,
            "Smaller value means activation can decrease more rapidly "
            "(units: seconds).");
    OpenSim_DECLARE_PROPERTY(default_activation, double,
            "Value of activation in the default state returned by "
            "initSystem().");
    OpenSim_DECLARE_PROPERTY(default_normalized_tendon_force, double,
            "Value of normalized tendon force in the default state returned by "
            "initSystem().");
    OpenSim_DECLARE_PROPERTY(active_force_width_scale, double,
            "Scale factor for the width of the active force-length curve. "
            "Larger values make the curve wider. Default: 1.0.");
    OpenSim_DECLARE_PROPERTY(fiber_damping, double,
            "The linear damping of the fiber. Default: 0.");
    OpenSim_DECLARE_PROPERTY(ignore_passive_fiber_force, bool,
            "Make the passive fiber force 0. Default: false.");
    OpenSim_DECLARE_PROPERTY(passive_fiber_strain_at_one_norm_force, double,
            "Fiber strain when the passive fiber force is 1 normalized force. "
            "Default: 0.6.");
    OpenSim_DECLARE_PROPERTY(tendon_strain_at_one_norm_force, double,
            "Tendon strain at a tension of 1 normalized force. "
            "Default: 0.049.");
    OpenSim_DECLARE_PROPERTY(tendon_compliance_dynamics_mode, std::string,
            "The dynamics method used to enforce tendon compliance dynamics. "
            "Options: 'explicit' or 'implicit'. Default: 'explicit'. ");

    OpenSim_DECLARE_OUTPUT(passive_fiber_elastic_force, double,
            getPassiveFiberElasticForce, SimTK::Stage::Dynamics);
    OpenSim_DECLARE_OUTPUT(passive_fiber_elastic_force_along_tendon, double,
            getPassiveFiberElasticForceAlongTendon, SimTK::Stage::Dynamics);
    OpenSim_DECLARE_OUTPUT(passive_fiber_damping_force, double,
            getPassiveFiberDampingForce, SimTK::Stage::Dynamics);
    OpenSim_DECLARE_OUTPUT(passive_fiber_damping_force_along_tendon, double,
            getPassiveFiberDampingForceAlongTendon, SimTK::Stage::Dynamics);

    OpenSim_DECLARE_OUTPUT(implicitresidual_normalized_tendon_force, double,
            getImplicitResidualNormalizedTendonForce, SimTK::Stage::Dynamics);
    OpenSim_DECLARE_OUTPUT(implicitenabled_normalized_tendon_force, bool,
            getImplicitEnabledNormalizedTendonForce, SimTK::Stage::Model);

    OpenSim_DECLARE_OUTPUT(statebounds_normalized_tendon_force, SimTK::Vec2,
            getBoundsNormalizedTendonForce, SimTK::Stage::Model);

    DeGrooteFregly2016Muscle() { constructProperties(); }

protected:
    //--------------------------------------------------------------------------
    // COMPONENT INTERFACE
    //--------------------------------------------------------------------------
    /// @name Component interface
    /// @{
    void extendFinalizeFromProperties() override;
    void extendAddToSystem(SimTK::MultibodySystem& system) const override;
    void extendInitStateFromProperties(SimTK::State& s) const override;
    void extendSetPropertiesFromState(const SimTK::State& s) override;
    void computeStateVariableDerivatives(const SimTK::State& s) const override;
    /// @}

    //--------------------------------------------------------------------------
    // ACTUATOR INTERFACE
    //--------------------------------------------------------------------------
    /// @name Actuator interface
    /// @{
    double computeActuation(const SimTK::State& s) const override;
    /// @}

public:
    //--------------------------------------------------------------------------
    // MUSCLE INTERFACE
    //--------------------------------------------------------------------------
    /// @name Muscle interface
    /// @{

    /// If ignore_activation_dynamics is true, this gets excitation instead.
    double getActivation(const SimTK::State& s) const override {
        // We override the Muscle's implementation because Muscle requires
        // realizing to Dynamics to access activation from MuscleDynamicsInfo,
        // which is unnecessary if the activation is a state.
        if (get_ignore_activation_dynamics()) {
            return getControl(s);
        } else {
            return getStateVariableValue(s, STATE_ACTIVATION_NAME);
        }
    }

    /// If ignore_activation_dynamics is true, this sets excitation instead.
    void setActivation(SimTK::State& s, double activation) const override {
        if (get_ignore_activation_dynamics()) {
            SimTK::Vector& controls(getModel().updControls(s));
            setControls(SimTK::Vector(1, activation), controls);
            getModel().setControls(s, controls);
        } else {
            setStateVariableValue(s, STATE_ACTIVATION_NAME, activation);
        }
        //markCacheVariableInvalid(s, "velInfo");
        //markCacheVariableInvalid(s, "dynamicsInfo");
    }

protected:
    double calcInextensibleTendonActiveFiberForce(
        SimTK::State&, double) const override;
    void calcMuscleLengthInfo(
            const SimTK::State& s, MuscleLengthInfo& mli) const override;
    void calcFiberVelocityInfo(
            const SimTK::State& s, FiberVelocityInfo& fvi) const override;
    void calcMuscleDynamicsInfo(
            const SimTK::State& s, MuscleDynamicsInfo& mdi) const override;
    void calcMusclePotentialEnergyInfo(const SimTK::State& s,
            MusclePotentialEnergyInfo& mpei) const override;

public:
    /// Fiber velocity is assumed to be 0.
    void computeInitialFiberEquilibrium(SimTK::State& s) const override;
    /// @}

    /// @name Get methods.
    /// @{

    /// Get the portion of the passive fiber force generated by the elastic
    /// element only (N).
    double getPassiveFiberElasticForce(const SimTK::State& s) const;
    /// Get the portion of the passive fiber force generated by the elastic
    /// element only, projected onto the tendon direction (N).
    double getPassiveFiberElasticForceAlongTendon(const SimTK::State& s) const;
    /// Get the portion of the passive fiber force generated by the damping
    /// element only (N).
    double getPassiveFiberDampingForce(const SimTK::State& s) const;
    /// Get the portion of the passive fiber force generated by the damping
    /// element only, projected onto the tendon direction (N).
    double getPassiveFiberDampingForceAlongTendon(const SimTK::State& s) const;

    /// We don't need the state, but the state parameter is a requirement of
    /// Output functions.
    bool getImplicitEnabledNormalizedTendonForce(const SimTK::State&) const {
        return !get_ignore_tendon_compliance() && !m_isTendonDynamicsExplicit;
    }
    /// Compute the muscle-tendon force equilibrium residual value when using
    /// implicit contraction dynamics with normalized tendon force as the
    /// state.
    double getImplicitResidualNormalizedTendonForce(
            const SimTK::State& s) const;

    /// If ignore_tendon_compliance is true, this gets normalized fiber force
    /// along the tendon instead.
    double getNormalizedTendonForce(const SimTK::State& s) const {
        if (get_ignore_tendon_compliance()) {
            return getTendonForce(s) / get_max_isometric_force();
        } else {
            return getStateVariableValue(s, STATE_NORMALIZED_TENDON_FORCE_NAME);
        }
    }

    /// If integration_mode is 'implicit', this gets the discrete variable
    /// tendon force derivative value. If integration_mode is 'explicit', this
    /// gets the value returned by getStateVariableDerivativeValue() for the
    /// 'normalized_tendon_force' state. If ignore_tendon_compliance is false,
    /// this returns zero.
    double getNormalizedTendonForceDerivative(const SimTK::State& s) const {
        if (get_ignore_tendon_compliance()) {
            return 0.0;
        } else {
            if (m_isTendonDynamicsExplicit) {
                return getStateVariableDerivativeValue(
                        s, STATE_NORMALIZED_TENDON_FORCE_NAME);
            } else {
                return getDiscreteVariableValue(
                        s, DERIVATIVE_NORMALIZED_TENDON_FORCE_NAME);
            }
        }
    }

    /// The residual (i.e. error) in the muscle-tendon equilibrium equation:
    ///         residual = tendonForce - fiberForce * cosPennationAngle
    double getEquilibriumResidual(const SimTK::State& s) const {
        const auto& mdi = getMuscleDynamicsInfo(s);
        return calcEquilibriumResidual(
                mdi.tendonForce, mdi.fiberForceAlongTendon);
    }

    /// The residual (i.e. error) in the time derivative of the linearized
    /// muscle-tendon equilibrium equation (Millard et al. 2013, equation A6):
    ///     residual = fiberStiffnessAlongTendon * fiberVelocityAlongTendon -
    ///                tendonStiffness *
    ///                    (muscleTendonVelocity - fiberVelocityAlongTendon)
    double getLinearizedEquilibriumResidualDerivative(
            const SimTK::State& s) const {
        const auto& muscleTendonVelocity = getLengtheningSpeed(s);
        const FiberVelocityInfo& fvi = getFiberVelocityInfo(s);
        const MuscleDynamicsInfo& mdi = getMuscleDynamicsInfo(s);

        return calcLinearizedEquilibriumResidualDerivative(muscleTendonVelocity,
                fvi.fiberVelocityAlongTendon, mdi.tendonStiffness,
                mdi.fiberStiffnessAlongTendon);
    }

    static std::string getActivationStateName() {
        return STATE_ACTIVATION_NAME;
    }
    static std::string getNormalizedTendonForceStateName() {
        return STATE_NORMALIZED_TENDON_FORCE_NAME;
    }
    static std::string getImplicitDynamicsDerivativeName() {
        return DERIVATIVE_NORMALIZED_TENDON_FORCE_NAME;
    }
    static std::string getImplicitDynamicsResidualName() {
        return RESIDUAL_NORMALIZED_TENDON_FORCE_NAME;
    }
    static double getMinNormalizedTendonForce() { return m_minNormTendonForce; }
    static double getMaxNormalizedTendonForce() { return m_maxNormTendonForce; }
    /// The first element of the Vec2 is the lower bound, and the second is the
    /// upper bound.
    /// We don't need the state, but the state parameter is a requirement of
    /// Output functions.
    SimTK::Vec2 getBoundsNormalizedTendonForce(const SimTK::State&) const
    { return {getMinNormalizedTendonForce(), getMaxNormalizedTendonForce()}; }
    /// @}

    /// @name Set methods.
    /// @{
    /// If ignore_tendon_compliance is true, this sets nothing.
    void setNormalizedTendonForce(
            SimTK::State& s, double normTendonForce) const {
        if (!get_ignore_tendon_compliance()) {
            setStateVariableValue(
                    s, STATE_NORMALIZED_TENDON_FORCE_NAME, normTendonForce);
            markCacheVariableInvalid(s, "lengthInfo");
            markCacheVariableInvalid(s, "velInfo");
            markCacheVariableInvalid(s, "dynamicsInfo");
        }
    }
    /// @}

    /// @name Calculation methods.
    /// These functions compute the values of normalized/dimensionless curves,
    /// their derivatives and integrals, and other quantities of the muscle.
    /// These do not depend on a SimTK::State.
    /// @{

    /// The active force-length curve is the sum of 3 Gaussian-like curves. The
    /// width of the curve can be adjusted via the active_force_width_scale
    /// property.
    SimTK::Real calcActiveForceLengthMultiplier(
            const SimTK::Real& normFiberLength) const {
        const double& scale = get_active_force_width_scale();
        // Shift the curve so its peak is at the origin, scale it
        // horizontally, then shift it back so its peak is still at x = 1.0.
        const double x = (normFiberLength - 1.0) / scale + 1.0;
        return calcGaussianLikeCurve(x, b11, b21, b31, b41) +
               calcGaussianLikeCurve(x, b12, b22, b32, b42) +
               calcGaussianLikeCurve(x, b13, b23, b33, b43);
    }

    /// The derivative of the active force-length curve with respect to
    /// normalized fiber length. This curve is based on the derivative of the
    /// Gaussian-like curve used in calcActiveForceLengthMultiplier(). The
    /// active_force_width_scale property also affects the value of the
    /// derivative curve.
    SimTK::Real calcActiveForceLengthMultiplierDerivative(
            const SimTK::Real& normFiberLength) const {
        const double& scale = get_active_force_width_scale();
        // Shift the curve so its peak is at the origin, scale it
        // horizontally, then shift it back so its peak is still at x = 1.0.
        const double x = (normFiberLength - 1.0) / scale + 1.0;
        return (1.0 / scale) *
               (calcGaussianLikeCurveDerivative(x, b11, b21, b31, b41) +
                       calcGaussianLikeCurveDerivative(x, b12, b22, b32, b42) +
                       calcGaussianLikeCurveDerivative(x, b13, b23, b33, b43));
    }

    /// The parameters of this curve are not modifiable, so this function is
    /// static.
    /// Domain: [-1, 1]
    /// Range: [0, 1.794]
    static SimTK::Real calcForceVelocityMultiplier(
            const SimTK::Real& normFiberVelocity) {
        using SimTK::square;
        const SimTK::Real tempV = d2 * normFiberVelocity + d3;
        const SimTK::Real tempLogArg = tempV + sqrt(square(tempV) + 1.0);
        return d1 * log(tempLogArg) + d4;
    }

    /// This is the inverse of the force-velocity multiplier function, and
    /// returns the normalized fiber velocity (in [-1, 1]) as a function of
    /// the force-velocity multiplier.
    static SimTK::Real calcForceVelocityInverseCurve(
            const SimTK::Real& forceVelocityMult) {
        // The version of this equation in the supplementary materials of De
        // Groote et al., 2016 has an error (it's missing a "-d3" before
        // dividing by "d2").
        return (sinh(1.0 / d1 * (forceVelocityMult - d4)) - d3) / d2;
    }

    /// This is the passive force-length curve. The curve becomes negative below
    /// the minNormFiberLength.
    ///
    /// We modified this equation from that in the supplementary materials of De
    /// Groote et al., 2016, which is the same function used in
    /// Thelen2003Muscle. The version in the supplementary materials passes
    /// through y = 0 at x = 1.0 and allows for negative forces. We do not want
    /// negative forces within the allowed range of fiber lengths, so we
    /// modified the equation to pass through y = 0 at x = 0.2. (This is not an
    /// issue for Thelen2003Muscle because the curve is not smooth, and returns
    /// 0 for lengths less than optimal fiber length.)
    SimTK::Real calcPassiveForceMultiplier(
            const SimTK::Real& normFiberLength) const {
        if (get_ignore_passive_fiber_force()) return 0;

        const double& e0 = get_passive_fiber_strain_at_one_norm_force();

        const double offset =
                exp(kPE * (m_minNormFiberLength - 1.0) / e0);
        const double denom = exp(kPE) - offset;

        return (exp(kPE * (normFiberLength - 1.0) / e0) - offset) / denom;
    }

    /// This is the derivative of the passive force-length curve with respect to
    /// the normalized fiber length.
    SimTK::Real calcPassiveForceMultiplierDerivative(
            const SimTK::Real& normFiberLength) const {

        if (get_ignore_passive_fiber_force()) return 0;

        const double& e0 = get_passive_fiber_strain_at_one_norm_force();

        const double offset = exp(kPE * (m_minNormFiberLength - 1) / e0);

        return (kPE * exp((kPE * (normFiberLength - 1)) / e0)) /
               (e0 * (exp(kPE) - offset));
    }

    /// This is the integral of the passive force-length curve with respect to
    /// the normalized fiber length.
    SimTK::Real calcPassiveForceMultiplierIntegral(
            const SimTK::Real& normFiberLength) const {

        if (get_ignore_passive_fiber_force()) return 0;

        const double& e0 = get_passive_fiber_strain_at_one_norm_force();

        const double temp1 = exp(kPE * m_minNormFiberLength / e0);
        const double denom = exp(kPE * (1.0 + 1.0 / e0)) - temp1;
        const double temp2 = kPE / e0 * normFiberLength;
        return (e0 / kPE * exp(temp2) - normFiberLength * temp1) / denom;
    }

    /// The normalized tendon force as a function of normalized tendon length.
    /// Note that this curve does not go through (1, 0); when
    /// normTendonLength=1, this function returns a slightly negative number.
    // TODO: In explicit mode, do not allow negative tendon forces?
    SimTK::Real calcTendonForceMultiplier(
            const SimTK::Real& normTendonLength) const {
        return c1 * exp(m_kT * (normTendonLength - c2)) - c3;
    }

    /// This is the derivative of the tendon-force length curve with respect to
    /// normalized tendon length.
    SimTK::Real calcTendonForceMultiplierDerivative(
            const SimTK::Real& normTendonLength) const {
        return c1 * m_kT * exp(m_kT * (normTendonLength - c2));
    }

    /// This is the integral of the tendon-force length curve with respect to
    /// normalized tendon length.
    SimTK::Real calcTendonForceMultiplierIntegral(
            const SimTK::Real& normTendonLength) const {
        return (c1 * exp(-m_kT * (c2 - normTendonLength))) / m_kT -
               c3 * normTendonLength;
    }

    /// This is the inverse of the tendon force-length curve, and returns the
    /// normalized tendon length as a function of the normalized tendon force.
    SimTK::Real calcTendonForceLengthInverseCurve(
            const SimTK::Real& normTendonForce) const {
        return log((1.0 / c1) * (normTendonForce + c3)) / m_kT + c2;
    }

    /// This is the derivative of the inverse tendon-force length. Given the
    /// derivative of normalized tendon force and normalized tendon length, this
    /// returns normalized tendon velocity.
    SimTK::Real calcTendonForceLengthInverseCurveDerivative(
            const SimTK::Real& derivNormTendonForce,
            const SimTK::Real& normTendonLength) const {
        return derivNormTendonForce /
               (c1 * m_kT * exp(m_kT * (normTendonLength - c2)));
    }

    /// This computes both the total fiber force and the individual components
    /// of fiber force (active, conservative passive, and non-conservative
    /// passive).
    /// @note based on Millard2012EquilibriumMuscle::calcFiberForce().
    void calcFiberForce(const SimTK::Real& activation,
            const SimTK::Real& activeForceLengthMultiplier,
            const SimTK::Real& forceVelocityMultiplier,
            const SimTK::Real& normPassiveFiberForce,
            const SimTK::Real& normFiberVelocity, SimTK::Real& activeFiberForce,
            SimTK::Real& conPassiveFiberForce,
            SimTK::Real& nonConPassiveFiberForce,
            SimTK::Real& totalFiberForce) const {
        const auto& maxIsometricForce = get_max_isometric_force();
        // active force
        activeFiberForce =
                maxIsometricForce * (activation * activeForceLengthMultiplier *
                                            forceVelocityMultiplier);
        // conservative passive force
        conPassiveFiberForce = maxIsometricForce * normPassiveFiberForce;
        // non-conservative passive force
        nonConPassiveFiberForce =
                maxIsometricForce * get_fiber_damping() * normFiberVelocity;
        // total force
        totalFiberForce = activeFiberForce + conPassiveFiberForce +
                          nonConPassiveFiberForce;
    }

    /// The stiffness of the fiber in the direction of the fiber. This includes
    /// both active and passive force contributions to stiffness from the muscle
    /// fiber.
    /// @note based on Millard2012EquilibriumMuscle::calcFiberStiffness().
    SimTK::Real calcFiberStiffness(const SimTK::Real& activation,
            const SimTK::Real& normFiberLength,
            const SimTK::Real& fiberVelocityMultiplier) const {

        const SimTK::Real partialNormFiberLengthPartialFiberLength =
                1.0 / get_optimal_fiber_length();
        const SimTK::Real partialNormActiveForcePartialFiberLength =
                partialNormFiberLengthPartialFiberLength *
                calcActiveForceLengthMultiplierDerivative(normFiberLength);
        const SimTK::Real partialNormPassiveForcePartialFiberLength =
                partialNormFiberLengthPartialFiberLength *
                calcPassiveForceMultiplierDerivative(normFiberLength);

        // fiberStiffness = d_fiberForce / d_fiberLength
        return get_max_isometric_force() *
               (activation * partialNormActiveForcePartialFiberLength *
                               fiberVelocityMultiplier +
                       partialNormPassiveForcePartialFiberLength);
    }

    /// The stiffness of the tendon in the direction of the tendon.
    /// @note based on Millard2012EquilibriumMuscle.
    SimTK::Real calcTendonStiffness(const SimTK::Real& normTendonLength) const {

        if (get_ignore_tendon_compliance()) return SimTK::Infinity;
        return (get_max_isometric_force() / get_tendon_slack_length()) *
               calcTendonForceMultiplierDerivative(normTendonLength);
    }

    /// The stiffness of the whole musculotendon unit in the direction of the
    /// tendon.
    /// @note based on Millard2012EquilibriumMuscle.
    SimTK::Real calcMuscleStiffness(const SimTK::Real& tendonStiffness,
            const SimTK::Real& fiberStiffnessAlongTendon) const {

        if (get_ignore_tendon_compliance()) return fiberStiffnessAlongTendon;
        // TODO Millard2012EquilibriumMuscle includes additional checks that
        // the stiffness is non-negative and that the denomenator is non-zero.
        return (fiberStiffnessAlongTendon * tendonStiffness) /
               (fiberStiffnessAlongTendon + tendonStiffness);
    }

    /// The derivative of pennation angle with respect to fiber length.
    /// @note based on
    /// MuscleFixedWidthPennationModel::calc_DPennationAngle_DFiberLength().
    SimTK::Real calcPartialPennationAnglePartialFiberLength(
            const SimTK::Real& fiberLength) const {

        using SimTK::square;
        // pennationAngle = asin(fiberWidth/fiberLength)
        // d_pennationAngle/d_fiberLength =
        //          d/d_fiberLength (asin(fiberWidth/fiberLength))
        return (-m_fiberWidth / square(fiberLength)) /
               sqrt(1.0 - square(m_fiberWidth / fiberLength));
    }

    /// The derivative of the fiber force along the tendon with respect to fiber
    /// length.
    /// @note based on
    /// Millard2012EquilibriumMuscle::calc_DFiberForceAT_DFiberLength().
    SimTK::Real calcPartialFiberForceAlongTendonPartialFiberLength(
            const SimTK::Real& fiberForce, const SimTK::Real& fiberStiffness,
            const SimTK::Real& sinPennationAngle,
            const SimTK::Real& cosPennationAngle,
            const SimTK::Real& partialPennationAnglePartialFiberLength) const {

        const SimTK::Real partialCosPennationAnglePartialFiberLength =
                -sinPennationAngle * partialPennationAnglePartialFiberLength;

        // The stiffness of the fiber along the direction of the tendon. For
        // small changes in length parallel to the fiber, this quantity is
        // d_fiberForceAlongTendon / d_fiberLength =
        //      d/d_fiberLength(fiberForce * cosPenneationAngle)
        return fiberStiffness * cosPennationAngle +
               fiberForce * partialCosPennationAnglePartialFiberLength;
    }

    /// The derivative of the fiber force along the tendon with respect to the
    /// fiber length along the tendon.
    /// @note based on
    /// Millard2012EquilibriumMuscle::calc_DFiberForceAT_DFiberLengthAT.
    SimTK::Real calcFiberStiffnessAlongTendon(const SimTK::Real& fiberLength,
            const SimTK::Real& partialFiberForceAlongTendonPartialFiberLength,
            const SimTK::Real& sinPennationAngle,
            const SimTK::Real& cosPennationAngle,
            const SimTK::Real& partialPennationAnglePartialFiberLength) const {

        // The change in length of the fiber length along the tendon.
        // fiberLengthAlongTendon = fiberLength * cosPennationAngle
        const SimTK::Real partialFiberLengthAlongTendonPartialFiberLength =
                cosPennationAngle -
                fiberLength * sinPennationAngle *
                        partialPennationAnglePartialFiberLength;

        // fiberStiffnessAlongTendon
        //    = d_fiberForceAlongTendon / d_fiberLengthAlongTendon
        //    = (d_fiberForceAlongTendon / d_fiberLength) *
        //      (1 / (d_fiberLengthAlongTendon / d_fiberLength))
        return partialFiberForceAlongTendonPartialFiberLength *
               (1.0 / partialFiberLengthAlongTendonPartialFiberLength);
    }

    SimTK::Real calcPartialTendonLengthPartialFiberLength(
            const SimTK::Real& fiberLength,
            const SimTK::Real& sinPennationAngle,
            const SimTK::Real& cosPennationAngle,
            const SimTK::Real& partialPennationAnglePartialFiberLength) const {

        return fiberLength * sinPennationAngle *
                       partialPennationAnglePartialFiberLength -
               cosPennationAngle;
    }

    SimTK::Real calcPartialTendonForcePartialFiberLength(
            const SimTK::Real& tendonStiffness, const SimTK::Real& fiberLength, 
            const SimTK::Real& sinPennationAngle, 
            const SimTK::Real& cosPennationAngle) const {
        const SimTK::Real partialPennationAnglePartialFiberLength =
                calcPartialPennationAnglePartialFiberLength(fiberLength);

        const SimTK::Real partialTendonLengthPartialFiberLength =
                calcPartialTendonLengthPartialFiberLength(fiberLength, 
                    sinPennationAngle, cosPennationAngle,
                        partialPennationAnglePartialFiberLength);

        return tendonStiffness * partialTendonLengthPartialFiberLength;
    }

    /// @copydoc getEquilibriumResidual()
    SimTK::Real calcEquilibriumResidual(const SimTK::Real& tendonForce,
            const SimTK::Real& fiberForceAlongTendon) const {

        return tendonForce - fiberForceAlongTendon;
    }

    /// @copydoc getLinearizedEquilibriumResidualDerivative()
    SimTK::Real calcLinearizedEquilibriumResidualDerivative(
            const SimTK::Real muscleTendonVelocity,
            const SimTK::Real& fiberVelocityAlongTendon,
            const SimTK::Real& tendonStiffness,
            const SimTK::Real& fiberStiffnessAlongTendon) const {

        return fiberStiffnessAlongTendon * fiberVelocityAlongTendon -
               tendonStiffness *
                       (muscleTendonVelocity - fiberVelocityAlongTendon);
    }
    /// @}

    /// @name Utilities
    /// @{

    /// Export the active force-length multiplier and passive force multiplier
    /// curves to a DataTable. If the normFiberLengths argument is omitted, we
    /// use createVectorLinspace(200, minNormFiberLength, maxNormFiberLength).
    DataTable exportFiberLengthCurvesToTable(
            const SimTK::Vector& normFiberLengths = SimTK::Vector()) const;
    /// Export the fiber force-velocity multiplier curve to a DataTable. If
    /// the normFiberVelocities argument is omitted, we use
    /// createVectorLinspace(200, -1.1, 1.1).
    DataTable exportFiberVelocityMultiplierToTable(
            const SimTK::Vector& normFiberVelocities = SimTK::Vector()) const;
    /// Export the fiber tendon force multiplier curve to a DataTable. If
    /// the normFiberVelocities argument is omitted, we use
    /// createVectorLinspace(200, 0.95, 1 + <strain at 1 norm force>)
    DataTable exportTendonForceMultiplierToTable(
            const SimTK::Vector& normTendonLengths = SimTK::Vector()) const;
    /// Print the muscle curves to STO files. The files will be named as
    /// `<muscle-name>_<curve_type>.sto`.
    ///
    /// @param directory
    ///     The directory to which the data files should be written. Do NOT
    ///     include the filename. By default, the files are printed to the
    ///     current working directory.
    void printCurvesToSTOFiles(const std::string& directory = ".") const;

    /// Replace muscles of other types in the model with muscles of this type.
    /// Currently, only Millard2012EquilibriumMuscles and Thelen2003Muscles
    /// are replaced. If the model has muscles of other types, an exception is
    /// thrown unless allowUnsupportedMuscles is true.
    /// Since the DeGrooteFregly2016Muscle implements tendon compliance dynamics
    /// with normalized tendon force as the state variable, this function
    /// ignores the 'default_fiber_length' property in replaced muscles.
    static void replaceMuscles(
            Model& model, bool allowUnsupportedMuscles = false);
    /// @}

private:
    void constructProperties();

    void calcMuscleLengthInfoHelper(const SimTK::Real& muscleTendonLength,
            const bool& ignoreTendonCompliance, MuscleLengthInfo& mli,
            const SimTK::Real& normTendonForce) const;
    void calcFiberVelocityInfoHelper(const SimTK::Real& muscleTendonVelocity,
            const SimTK::Real& activation, const bool& ignoreTendonCompliance,
            const bool& isTendonDynamicsExplicit,
            const MuscleLengthInfo& mli, FiberVelocityInfo& fvi,
            const SimTK::Real& normTendonForce,
            const SimTK::Real& normTendonForceDerivative) const;
    void calcMuscleDynamicsInfoHelper(const SimTK::Real& activation,
            const SimTK::Real& muscleTendonVelocity,
            const bool& ignoreTendonCompliance, const MuscleLengthInfo& mli,
            const FiberVelocityInfo& fvi, MuscleDynamicsInfo& mdi,
            const SimTK::Real& normTendonForce) const;
    void calcMusclePotentialEnergyInfoHelper(const bool& ignoreTendonCompliance,
            const MuscleLengthInfo& mli, MusclePotentialEnergyInfo& mpei) const;

    /// This is a Gaussian-like function used in the active force-length curve.
    /// A proper Gaussian function does not have the variable in the denominator
    /// of the exponent.
    /// The supplement for De Groote et al., 2016 has a typo:
    /// the denominator should be squared.
    static SimTK::Real calcGaussianLikeCurve(const SimTK::Real& x,
            const double& b1, const double& b2, const double& b3,
            const double& b4) {
        using SimTK::square;
        return b1 * exp(-0.5 * square(x - b2) / square(b3 + b4 * x));
    }

    /// The derivative of the curve defined in calcGaussianLikeCurve() with
    /// respect to 'x' (usually normalized fiber length).
    static SimTK::Real calcGaussianLikeCurveDerivative(const SimTK::Real& x,
            const double& b1, const double& b2, const double& b3,
            const double& b4) {
        using SimTK::cube;
        using SimTK::square;
        return (b1 * exp(-square(b2 - x) / (2 * square(b3 + b4 * x))) *
                       (b2 - x) * (b3 + b2 * b4)) /
               cube(b3 + b4 * x);
    }

    enum StatusFromEstimateMuscleFiberState {
        Success_Converged,
        Warning_FiberAtLowerBound,
        Warning_FiberAtUpperBound,
        Failure_MaxIterationsReached
    };

    struct ValuesFromEstimateMuscleFiberState {
        int iterations;
        double solution_error;
        double fiber_length;
        double fiber_velocity;
        double normalized_tendon_force;
    };

    std::pair<StatusFromEstimateMuscleFiberState,
            ValuesFromEstimateMuscleFiberState>
    estimateMuscleFiberState(const double activation, 
            const double muscleTendonLength, const double muscleTendonVelocity, 
            const double normTendonForceDerivative, const double tolerance,
            const int maxIterations) const;

    // Curve parameters.
    // Notation comes from De Groote et al., 2016 (supplement).

    // Parameters for the active fiber force-length curve.
    // ---------------------------------------------------
    // Values are taken from https://simtk.org/projects/optcntrlmuscle
    // rather than the paper supplement. b11 was modified to ensure that
    // f(1) = 1.
    constexpr static double b11 = 0.8150671134243542;
    constexpr static double b21 = 1.055033428970575;
    constexpr static double b31 = 0.162384573599574;
    constexpr static double b41 = 0.063303448465465;
    constexpr static double b12 = 0.433004984392647;
    constexpr static double b22 = 0.716775413397760;
    constexpr static double b32 = -0.029947116970696;
    constexpr static double b42 = 0.200356847296188;
    constexpr static double b13 = 0.1;
    constexpr static double b23 = 1.0;
    constexpr static double b33 = 0.353553390593274; // 0.5 * sqrt(0.5)
    constexpr static double b43 = 0.0;

    // Parameters for the passive fiber force-length curve.
    // ---------------------------------------------------
    // Exponential shape factor.
    constexpr static double kPE = 4.0;

    // Parameters for the tendon force curve.
    // --------------------------------------
    constexpr static double c1 = 0.200;
    // Horizontal asymptote as x -> -inf is -c3.
    // Normalized force at 0 strain is c1 * exp(-c2) - c3.
    // This parameter is 0.995 in De Groote et al., which causes the y-value at
    // 0 strain to be negative. We use 1.0 so that the y-value at 0 strain is 0
    // (since c2 == c3).
    constexpr static double c2 = 1.0;
    // This parameter is 0.250 in De Groote et al., which causes
    // lim(x->-inf) = -0.25 instead of -0.20.
    constexpr static double c3 = 0.200;

    // Parameters for the force-velocity curve.
    // ----------------------------------------
    // The parameters from the paper supplement are rounded/truncated and cause
    // the curve to not go through the points (-1, 0) and (0, 1). We solved for
    // different values of d1 and d4 so that the curve goes through (-1, 0) and
    // (0, 1).
    // The values from the code at https://simtk.org/projects/optcntrlmuscle
    // also do not go through (-1, 0) and (0, 1).
    constexpr static double d1 = -0.3211346127989808;
    constexpr static double d2 = -8.149;
    constexpr static double d3 = -0.374;
    constexpr static double d4 = 0.8825327733249912;

    constexpr static double m_minNormFiberLength = 0.2;
    constexpr static double m_maxNormFiberLength = 1.8;

    constexpr static double m_minNormTendonForce = 0.0;
    constexpr static double m_maxNormTendonForce = 5.0;

    static const std::string STATE_ACTIVATION_NAME;
    static const std::string STATE_NORMALIZED_TENDON_FORCE_NAME;
    static const std::string DERIVATIVE_NORMALIZED_TENDON_FORCE_NAME;
    static const std::string RESIDUAL_NORMALIZED_TENDON_FORCE_NAME;

    // Computed from properties.
    // -------------------------

    // The square of (fiber_width / optimal_fiber_length).
    SimTK::Real m_fiberWidth = SimTK::NaN;
    SimTK::Real m_squareFiberWidth = SimTK::NaN;
    SimTK::Real m_maxContractionVelocityInMetersPerSecond = SimTK::NaN;
    // Tendon stiffness parameter from De Groote et al., 2016. Instead of
    // kT, users specify tendon strain at 1 norm force, which is more intuitive.
    SimTK::Real m_kT = SimTK::NaN;
    bool m_isTendonDynamicsExplicit = true;

    // Indices for MuscleDynamicsInfo::userDefinedDynamicsExtras.
    constexpr static int m_mdi_passiveFiberElasticForce = 0;
    constexpr static int m_mdi_passiveFiberDampingForce = 1;
    constexpr static int m_mdi_partialPennationAnglePartialFiberLength = 2;
    constexpr static int m_mdi_partialFiberForceAlongTendonPartialFiberLength =
            3;
    constexpr static int m_mdi_partialTendonForcePartialFiberLength = 4;
};

} // namespace OpenSim

#endif // OPENSIM_DEGROOTEFREGLY2016MUSCLE_H
