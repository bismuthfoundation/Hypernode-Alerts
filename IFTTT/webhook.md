# Trigger an HTTP request to Text Message from IFTTT

Everything [IFTTT](https://ifttt.com) can do is accomplished through applets. These are tiny programs that you can create which use triggers (the “If’s”) to execute actions (the “Then That’s”).

This example shows how to create IFTTT Webhook Applet that work with any device or app that can make a web request and triggered IFTTT Webhook Applet to sends a text message.

## Create an IFTTT Applet

You can set the IFTTT webhooks service to use web requests to trigger an action. The incoming action is an HTTP request to the web server and the outgoing action is a text message.

1. Create an IFTTT account, or log into your existing account.
2. Create an Applet. Click on **My Applets** at the top of the screen, then click the **New Applet**  button in the upper right.
3. You’ll then be taken to a screen that features the words “if +this then that.” You’ll need to click on the blue **+this** section of the text.

![IFTTT_this](images/this.png?raw=true "Optional Title")

4. Select the Webhooks service. Enter *Webhooks* in the search field. Select the Webhooks card.
5. Complete the trigger fields. After you select Webhooks as the trigger, click the Receive a web request card to continue. Enter an event name. This example uses *HN-BIS* as the event name. Click **Create trigger**.

![IFTTT_this](images/event_trigger.png?raw=true "Optional Title")

6. It’s time to select the action you want to have activated by your trigger. To do that, click on **+that** which will now be blue. The trigger word this is now the Webhooks icon. Enter SMS in the search bar, and select the SMS box.

![IFTTT_this](images/choose_sms.png?raw=true "Optional Title")

7. Select Send me an SMS, and then enter the text message information. You can pass data about the event that triggered your message using ingredients. For example, including {OccurredAt} adds the  Date & Time value to your text message. Click **Create action** to finish the new Applet. 

![IFTTT_this](images/sms_action_fields.png?raw=true "Optional Title")

Then, click **Finish** and you’ll be all set.

![IFTTT_this](images/sms_webhook.png?raw=true "Optional Title")

To retrieve your Webhooks trigger information, Select **My Applets > Services**, and search for Webhooks. Select Webhooks, and then click the Documentation button. You see your key and the format for sending a request. Enter the event name. The event name for this example is HN-BIS.

    https://maker.ifttt.com/trigger/{event}/with/key/XXXXXXXXXXXXXXXXXXXXXX

    https://maker.ifttt.com/trigger/HN-BIS/with/key/XXXXXXXXXXXXXXXXXXXXXX

You can test the service using the test button or by pasting the URL into your browser. The IFTTT event trigger is not always instantaneous

## Turning Applet on and off

Once you’ve activated one or more applets, you can turn them on and off as needed. Simply click on the **My Applets** text at the top of the screen, click onto the applet you’re interested in controlling, and click the “On” button to turn it to “off.”

On these screens, you can also make changes to any applets you’ve created. To do so, click the gear icon in the upper-right corner of the applet box and you’ll see the options you have for applet customization.

At any time, click the IFTTT logo in the upper-left corner of the screen to be taken back to the main page.
