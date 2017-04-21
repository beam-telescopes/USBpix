
/**************************************************************************
                          kuintspinbox.h  -  description
                             -------------------
    begin                : Sat Apr 12 2003
    copyright            : (C) 2003 by German Martinez
    email                : german.martinez@cern.ch
 ***************************************************************************/

/**
 *  An unsigned integer inputline with scrollbar and slider.
 *
 *  The class provides an easy interface to use other
 *  numeric systems then the decimal.
 *
 *  @short A spin box widget for non-decimal numbers.
 */

#ifndef USPINBOX_H
#define USPINBOX_H

#include <QSpinBox>

class QValidator;

class USpinBox : public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY( int base READ base WRITE setBase )

public:

    /**
     *  Constructor.
     *
     *  Constructs a widget with an integer inputline with a little scrollbar
     *  and a slider, with minimal value 0, maximal value 99, step 1, base 10
     *  and initial value 0.
     */
    USpinBox( QWidget *parent=0);
    /**
     *  Constructor.
     *
     *  Constructs a widget with an integer inputline with a little scrollbar
     *  and a slider.
     *
     *  @param lower  The lowest valid value.
     *  @param upper  The greatest valid value.
     *  @param step   The step size of the scrollbar.
     *  @param value  The actual value.
     *  @param base   The base of the used number system.
     *  @param parent The parent of the widget.
     *  @param name   The Name of the widget.
     */
    USpinBox(unsigned long lower, unsigned long upper, int step, unsigned long value, int base = 16,
                QWidget* parent = 0);

    /**
     *  Destructor.
     */
    virtual ~USpinBox();


    /**
     * Sets the base in which the numbers in the spin box are represented.
     */
    void setBase(int base);
    /**
     * @return the base in which numbers in the spin box are represented.
     */
    int base() const;

    inline int UtoI(unsigned long U) const { return U - 2147483648;}
    inline unsigned long ItoU(int I) const { return I + 2147483648;}

    inline unsigned long umin() const {return ItoU(minimum()); };
    inline unsigned long umax() const {return ItoU(maximum()); };

    inline void setUmin(unsigned long umin){setMinimum(UtoI(umin));};
    inline void setUmax(unsigned long umax){setMaximum(UtoI(umax));};

    unsigned long UValue();
    void setUValue(unsigned long);

    //protected:

    QString textFromValue(int) const;
    int valueFromText(const QString &) const;
    QValidator::State validate ( QString & input, int & pos ) const;

private:
    int m_valBase;
    unsigned long m_val;
};

/* --------------------------------------------------------------------------- */


#endif













