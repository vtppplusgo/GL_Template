#ifndef RawController_h
#define RawController_h

#include "Controller.hpp"
#include "../../Common.hpp"
#include <map>

/**
 \brief Represents a controller used for debug, where all raw buttons are shown.
 \ingroup Input
 */
class RawController : public Controller {

public:
	
	/// Constructor
	RawController();
	
	/**
	 \copydoc Controller::activate
	 */
	bool activate(int id);
	
	/**
	 \copydoc Controller::deactivate
	 */
	void deactivate();
	
	/**
	 \copydoc Controller::update
	 */
	void update();
	
	/** Query the name of the controller.
	 \return the name
	 */
	std::string name() const { return _name; }
	
	std::vector<float> allAxes; ///< Raw intensity values on all axes.
	std::vector<Controller::ControllerButton> allButtons; ///< Mapping of each button.
	
private:
	std::string _name; ///< Name of the joystick
	int _rawAxesCount = 0; ///< Number of axes returned by GLFW
	int _rawButtonsCount = 0; ///< Number of buttons returned by GLFW
	
	
};

#endif
